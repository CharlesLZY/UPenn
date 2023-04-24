/**
 * @file interface.c
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "interface.h"
#include "filesys.h"

extern FATConfig *fs_FATConfig;
extern uint16_t *fs_FAT16InMemory;
extern FdTable fs_fdTable;

/* global variable from kernel */
extern pcb *active_process;

int f_open(const char *fname, int mode) {
    if (isFileSystemMounted() == false) {
        printf("Error: No mounted file system.\n");
        return F_FAILURE;
    }

    if (isValidFileName(fname) == false) {
        printf("Error: Fail to open. Invalid filename %s.\n", fname);
        return F_FAILURE;
    }

    int fd = findAvailableFd(active_process->fds);
    if (fd == FS_NOT_FOUND) {
        printf("Error: Fail to open. Out of file descriptor. \n");
        return F_FAILURE;
    }

    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fname);
    int fileOffset;

    if (mode == F_READ) {
        if (directoryEntryOffset == FS_NOT_FOUND) {
            #ifdef FS_DEBUG_INFO
            printf("File %s is not found.\n", fname);
            #endif
            return F_FAILURE;
        }

        if (isFileBeingWritten(&fs_fdTable, directoryEntryOffset) == true) {
            #ifdef FS_DEBUG_INFO
            printf("Fail to open with F_READ. File %s is being written.\n", fname);
            #endif
            return F_FAILURE;
        }

        DirectoryEntry dir;
        readDirectoryEntry(fs_FATConfig, directoryEntryOffset, &dir);
        fileOffset = fs_FATConfig->FATRegionSize + (dir.firstBlock - 1) * fs_FATConfig->blockSize;

    } else if (mode == F_WRITE) {
        if (directoryEntryOffset != FS_NOT_FOUND) {
            if (isFileBeingUsed(&fs_fdTable, directoryEntryOffset) == true) {
                #ifdef FS_DEBUG_INFO
                printf("Fail to open with F_WRITE. File %s is being used.\n", fname);
                #endif
                return F_FAILURE;
            }
            deleteFileDirectoryByName(fs_FATConfig, fs_FAT16InMemory, fname); // truncate the file
        }
        directoryEntryOffset = fs_touch(fname);
        fileOffset = 0;

    } else if (mode == F_APPEND) { 
        if (directoryEntryOffset != FS_NOT_FOUND) {
            if (isFileBeingUsed(&fs_fdTable, directoryEntryOffset) == true) {
                #ifdef FS_DEBUG_INFO
                printf("Fail to open with F_WRITE. File %s is being used.\n", fname);
                #endif
                return F_FAILURE;
            }
        }
        directoryEntryOffset = fs_touch(fname);
        fileOffset = traceFileEnd(fs_FATConfig, fs_FAT16InMemory, fname);

    } else {
        printf("Error: Invalid open mode.\n");
        return F_FAILURE;
    }

    FdNode *newNode = createFdNode(mode, directoryEntryOffset, fileOffset);
    appendFdTable(&fs_fdTable, newNode);
    active_process->fds[fd] = newNode;

    return fd;
}

int f_close(int fd) {
    
    if (isFileSystemMounted() == false) {
        printf("Error: No mounted file system.\n");
        return F_FAILURE;
    }

    if ((fd < F_MIN_FD) || (fd >= MAX_FILE_DESCRIPTOR)) {
        printf("Error: Invalid file descriptor %d.\n", fd);
        return F_FAILURE;
    }

    FdNode *fdNode = active_process->fds[fd];
    if (fdNode == NULL) {
        return F_FAILURE;
    }

    int directoryEntryOffset = fdNode->directoryEntryOffset;
    if ((isFileBeingUsed(&fs_fdTable, directoryEntryOffset) == false) && (isDirectoryEntryToDelete(fs_FATConfig, directoryEntryOffset) == true)) {
        deleteFileDirectory(fs_FATConfig, fs_FAT16InMemory, directoryEntryOffset);
    }

    removeFdNode(active_process->fds[fd]);
    active_process->fds[fd] = NULL;
    return F_SUCCESS;
}

int f_read(int fd, int n, char *buf) {
    if (isFileSystemMounted() == false) {
        printf("Error: No mounted file system.\n");
        return F_FAILURE;
    }

    if ((fd < 0) || (fd >= MAX_FILE_DESCRIPTOR)) {
        printf("Error: Invalid file descriptor %d.\n", fd);
        return F_FAILURE;
    }

    FdNode *fdNode = active_process->fds[fd];
    if (fdNode == NULL) {
        if (fd == F_STDIN_FD) {
            int res = read(fd, buf, n);
            if (res == -1) {
                printf("Error: Fail to read from stdin.\n");
                return F_FAILURE;
            }
            return res;
        } else if ((fd == F_STDOUT_FD) || (fd == F_ERROR)) {
            printf("Error: Invalid file descriptor %d.\n", fd);
            return F_FAILURE;
        } else {
            printf("Error: Fail to read. No open file with fd %d.\n", fd);
            return F_FAILURE;
        }
    }

    if (fdNode->fileOffset == 0) { // empty file
        return 0;
    }

    int bytesToEnd = traceBytesToEnd(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fdNode->fileOffset);
    int bytesToRead = (n > bytesToEnd) ? bytesToEnd : n;

    int blockSize = fs_FATConfig->blockSize;
    int startBlock = fdNode->fileOffset / blockSize;
    int startBlockOffset = fdNode->fileOffset % blockSize;

    int bytesRead = fs_readFAT(startBlock, startBlockOffset, bytesToRead, buf);
    fdNode->fileOffset = traceOffset(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fdNode->fileOffset, bytesRead);

    return bytesRead;
}


int f_write(int fd, const char *str, int n) {
    if (isFileSystemMounted() == false) {
        printf("Error: No mounted file system.\n");
        return F_FAILURE;
    }

    if ((fd < 0) || (fd >= MAX_FILE_DESCRIPTOR)) {
        printf("Error: Invalid file descriptor %d.\n", fd);
        return F_FAILURE;
    }

    FdNode *fdNode = active_process->fds[fd];
    if (fdNode == NULL) {
        if (fd == F_STDIN_FD) {
            printf("Error: Invalid file descriptor %d.\n", fd);
            return F_FAILURE;
        } else if ((fd == F_STDOUT_FD) || (fd == F_ERROR)) {
            int res = write(fd, str, n);
            if (res == -1) {
                printf("Error: Fail to write to stdout.\n");
                return F_FAILURE;
            }
            return res;
        } else {
            printf("Error: Fail to write. No open file with fd %d.\n", fd);
            return F_FAILURE;
        }
    }

    if (fdNode->openMode == F_READ) {
        printf("Error: Invalid open mode.\n");
        return F_FAILURE;
    }

    if (n == 0) { // 0 byte to write
        return 0;
    }

    int dataRegionOffset = fs_FATConfig->FATRegionSize;
    int blockSize = fs_FATConfig->blockSize;
    int startBlock;
    int startBlockOffset;

    DirectoryEntry dir;
    readDirectoryEntry(fs_FATConfig, fdNode->directoryEntryOffset, &dir);

    if (fdNode->fileOffset == 0) { // empty file
        /* assign a data block for the empty file */
        startBlock = findEmptyFAT16Entry(fs_FATConfig, fs_FAT16InMemory);
        if (startBlock == FS_NOT_FOUND) {
            printf("Error: Fail to write. No empty data block.\n");
            return F_FAILURE;
        }

        startBlockOffset = 0;
        fs_FAT16InMemory[startBlock] = NO_SUCC_FAT_ENTRY;
        dir.firstBlock = (uint16_t) startBlock;
        writeFileDirectory(fs_FATConfig, fdNode->directoryEntryOffset, &dir);

        fdNode->fileOffset = dataRegionOffset + (startBlock - 1) * blockSize;
    } else {
        startBlock = fdNode->fileOffset / blockSize;
        startBlockOffset = fdNode->fileOffset % blockSize;
    }

    int bytesToEnd = traceBytesToEnd(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fdNode->fileOffset);
    int bytesWritten = fs_writeFAT(startBlock, startBlockOffset, n, str);

    if (bytesWritten > bytesToEnd) {
        dir.size += bytesWritten - bytesToEnd;
        writeFileDirectory(fs_FATConfig, fdNode->directoryEntryOffset, &dir);
    }

    if (bytesWritten != -1) {
        fdNode->fileOffset = traceOffset(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fdNode->fileOffset, bytesWritten);
    }

    return bytesWritten;
}

int f_lseek(int fd, int offset, int whence) {
    if (isFileSystemMounted() == false) {
        printf("Error: No mounted file system.\n");
        return F_FAILURE;
    }

    if ((fd < F_MIN_FD) || (fd >= MAX_FILE_DESCRIPTOR)) {
        printf("Error: Invalid file descriptor %d.\n", fd);
        return F_FAILURE;
    }

    if (offset < 0) {
        printf("Error: Invalid offset %d.\n", offset);
        return F_FAILURE;
    }

    FdNode *fdNode = active_process->fds[fd];
    if (fdNode == NULL) {
        printf("Error: Fail to lseek. No open file with fd %d.\n", fd);
        return F_FAILURE;
    }

    if (fdNode->fileOffset == 0) { // empty file
        return 0;
    }

    int dataRegionOffset = fs_FATConfig->FATRegionSize;
    int blockSize = fs_FATConfig->blockSize;

    DirectoryEntry dir;
    readDirectoryEntry(fs_FATConfig, fdNode->directoryEntryOffset, &dir);
    if (whence == F_SEEK_SET) {
        if (fdNode->openMode != F_APPEND) {
            int startOffset = dataRegionOffset + (dir.firstBlock - 1) * blockSize;
            int res = traceOffset(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, startOffset, offset);
            fdNode->fileOffset = res;   
        }

        return traceBytesFromBeginning(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fdNode->fileOffset);
        
    } else if (whence == F_SEEK_CUR) {
        int res = traceOffset(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fdNode->fileOffset, offset);
        fdNode->fileOffset = res;

        return traceBytesFromBeginning(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fdNode->fileOffset);

    } else if (whence == F_SEEK_END) {
        int fileEndOffset = traceFileEnd(fs_FATConfig, fs_FAT16InMemory, dir.name);
        int res = traceOffset(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fileEndOffset, offset);
        fdNode->fileOffset = res;

        return traceBytesFromBeginning(fs_FATConfig, fs_FAT16InMemory, fdNode->directoryEntryOffset, fdNode->fileOffset);
        
    } else {
        printf("Error: Fail to lseek. Invalid flag.\n");
        return F_FAILURE;
    }
}

int f_unlink(const char *fname) {
    if (isFileSystemMounted() == false) {
        printf("Error: No mounted file system.\n");
        return F_FAILURE;
    }

    if (isValidFileName(fname) == false) {
        printf("Error: Fail to open. Invalid filename %s.\n", fname);
        return F_FAILURE;
    }

    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fname);
    if (directoryEntryOffset == FS_NOT_FOUND) {
        return F_FAILURE;
    }

    if (isFileBeingUsed(&fs_fdTable, directoryEntryOffset) == true) {
        /* Mark the directory entry as deleted file in use */
        int fd = open(fs_FATConfig->name, O_WRONLY);
        lseek(fd, directoryEntryOffset, SEEK_SET);
        write(fd, DELETED_DIRECTORY_IN_USE, sizeof(char));
        close(fd);

    } else {
        int res = deleteFileDirectory(fs_FATConfig, fs_FAT16InMemory, directoryEntryOffset);
        if (res == FS_FAILURE) {
            printf("Error: Fail to delete file %s\n", fname);
            return F_FAILURE;
        }
    }
    return F_SUCCESS;
}

int f_ls(const char *filename) {
    if (isFileSystemMounted() == false) {
        printf("Error: No mounted file system.\n");
        return F_FAILURE;
    }
    if (filename != NULL) {
        if (isValidFileName(filename) == false) {
            printf("Error: Fail to open. Invalid filename %s.\n", filename);
            return F_FAILURE;
        }

        int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, filename);
        if (directoryEntryOffset == FS_NOT_FOUND) {
            printf("Error: Fail to ls %s. File not found.\n", filename);
            return F_FAILURE;
        }

        DirectoryEntry dir;
        readDirectoryEntry(fs_FATConfig, directoryEntryOffset, &dir);
        char perm[4] = {0};
        perm[0] = (((dir.perm & 0b100) >> 2) == 1) ? 'r' : '-';
        perm[1] = (((dir.perm & 0b010) >> 1) == 1) ? 'w' : '-';
        perm[2] = ((dir.perm & 0b001) == 1) ? 'x' : '-';

        struct tm *mtime = localtime(&dir.mtime); 
        char timeBuffer[32];
        strftime(timeBuffer, 32, "%b %d %H:%M", mtime);
        printf("%5d  %s %*d %s %s\n", dir.firstBlock, perm, 5+fs_FATConfig->LSB, dir.size, timeBuffer, dir.name);
    } else {
        int dataRegionOffset = fs_FATConfig->FATRegionSize;
        int blockSize = fs_FATConfig->blockSize;
        uint16_t curBlock = 1; // root directory block

        DirectoryEntry dir;
        while (curBlock != NO_SUCC_FAT_ENTRY) {
            /* Traverse all directory blocks and print each file directory entry. */
            for (int i = 0; i < blockSize / DIRECTORY_ENTRY_SIZE; i++) {
                int offset = dataRegionOffset + (curBlock - 1) * blockSize + DIRECTORY_ENTRY_SIZE * i;
                readDirectoryEntry(fs_FATConfig, offset, &dir);
                
                if (dir.name[0] == DIRECTORY_END[0]) {
                    break;
                }

                if ((dir.name[0] == DELETED_DIRECTORY[0]) || (dir.name[0] == DELETED_DIRECTORY_IN_USE[0])) {
                    continue;
                }

                char perm[4] = {0};
                perm[0] = (((dir.perm & 0b100) >> 2) == 1) ? 'r' : '-';
                perm[1] = (((dir.perm & 0b010) >> 1) == 1) ? 'w' : '-';
                perm[2] = ((dir.perm & 0b001) == 1) ? 'x' : '-';

                struct tm *mtime = localtime(&dir.mtime); 
                char timeBuffer[32];
                strftime(timeBuffer, 32, "%b %d %H:%M", mtime);
                printf("%5d  %s %*d %s %s\n", dir.firstBlock, perm, 5+fs_FATConfig->LSB, dir.size, timeBuffer, dir.name);
            }

            curBlock = fs_FAT16InMemory[curBlock];
        }
    }
    return F_SUCCESS;
}

bool f_find(const char *filename) {
    int res = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, filename);
    if (res == FS_NOT_FOUND) {
        return false;
    }
    return true;
}

bool f_isExecutable(const char *filename) {
    int res = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, filename);
    if (res == FS_NOT_FOUND) {
        return false;
    }
    DirectoryEntry dir;
    readDirectoryEntry(fs_FATConfig, res, &dir);
    if (dir.perm != FILE_PERM_READ_EXEC && dir.perm != FILE_PERM_READ_WRITE_EXEC) {
        return false;
    } else {
        return true;
    }
}