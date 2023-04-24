/**
 * @file filesys.c
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "filesys.h"

FATConfig *fs_FATConfig = NULL;
/* 
The FAT region will be mapped to the memory through mmap(). 
For the modifications of the file mapped by mmap(), they may not be written back to the file immediately because of the virtual memory management mechanism. 
Here are some situations that will trigger the write back:
1. msync()
2. mummap()
3. Out of memory
4. Periodic write back by OS

So the consistency of PennFAT may not be ensured.
*/
uint16_t *fs_FAT16InMemory = NULL;

FdTable fs_fdTable;

bool isFileSystemMounted() {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        return false;
    } else {
        return true;
    }
}

bool isValidFileName(const char *fileName) {
    if ((strlen(fileName) > MAX_FILE_NAME_LENGTH) || (strlen(fileName) == 0)) {
        return false;
    }

    if ((fileName[0] == DIRECTORY_END[0]) || (fileName[0] == DELETED_DIRECTORY[0]) || (fileName[0] == DELETED_DIRECTORY_IN_USE[0])) {
        return false;
    }

    return true;
}

int fs_mkfs(const char *fsName, uint16_t blockSizeConfig, uint16_t FATRegionBlockNum) {
    FATConfig *config = createFATConfig(fsName, blockSizeConfig, FATRegionBlockNum);
    int res = createFATOnDisk(config);
    if (res == FS_FAILURE) {
        printf("Error: Fail to create FAT on disk.\n");
    }

    #ifdef FS_DEBUG_INFO
    printf("File system %s is created.\n", fsName);
    printf("FAT Spec:\n- Block size: %d\n- FAT region: %d-byte\n- Data region: %d-byte\n", config->blockSize, config->FATRegionSize, config->dataRegionSize);
    #endif

    free(config);
    return res;
}

int fs_mount(const char *fsName) {
    int fd = open(fsName, O_RDWR);
    if (fd == -1) {
        printf("Error: Fail to open the file system %s.\n", fsName);
        return FS_FAILURE;
    }

    uint16_t FATSpec;
    read(fd, &FATSpec, 2);

    uint16_t LSB = FATSpec & 0x00FF;
    uint16_t MSB = FATSpec >> 8;
    fs_FATConfig = createFATConfig(fsName, LSB, MSB);

    /* Notice that FAT size in memory >= FAT region size. */
    fs_FAT16InMemory = mmap(NULL, fs_FATConfig->FATSizeInMemory, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // We use MAP_SHARED flag because we may want to examine the FAT through other tool when filesystem is running
    if (fs_FAT16InMemory == MAP_FAILED) {
        printf("Error: Fail to mmap FAT from filesystem %s.\n", fsName);
        return FS_FAILURE;
    }

    initFdTable(&fs_fdTable);

    #ifdef FS_DEBUG_INFO
    printf("File system %s is mounted.\n", fsName);
    printf("FAT Spec:\n- Block size: %d\n- FAT region: %d-byte\n- Data region: %d-byte\n", fs_FATConfig->blockSize, fs_FATConfig->FATRegionSize, fs_FATConfig->dataRegionSize);
    #endif

    return FS_SUCCESS;
}

int fs_unmount() {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system. Fail to unmount the file system.\n");
        return FS_FAILURE;
    }

    #ifdef FS_DEBUG_INFO
    printf("%s is unmounted.\n", fs_FATConfig->name);
    #endif

    free(fs_FATConfig);
    munmap(fs_FAT16InMemory, fs_FATConfig->FATSizeInMemory);
    fs_FATConfig = NULL;
    fs_FAT16InMemory = NULL;

    clearFdTable(&fs_fdTable);

    return FS_SUCCESS;
}

int fs_touch(const char *fileName) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if (isValidFileName(fileName) == false) {
        printf("Error: Invalid filename %s.\n", fileName);
        return FS_FAILURE;
    }

    int res = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (res == FS_NOT_FOUND) {
        /* Create the file directory. */
        res = createFileDirectoryOnDisk(fs_FATConfig, fs_FAT16InMemory, fileName, FILE_TYPE_REGULAR, FILE_PERM_READ_WRITE);
        if (res == FS_FAILURE) {
            printf("Error: Fail to touch a new file.\n");
            return res;
        }

        #ifdef FS_DEBUG_INFO
        printf("Empty file %s is created.\n", fileName);
        #endif

    } else {
        /* Update the time stamp. */
        int fd = open(fs_FATConfig->name, O_WRONLY);
        int offset = res + 32 + 4 + 2 + 1 + 1;
        lseek(fd, offset, SEEK_SET);
        time_t mtime = time(NULL);
        write(fd, &mtime, sizeof(time_t));
        close(fd);

        #ifdef FS_DEBUG_INFO
        printf("The time stamp of file %s is updated to %s", fileName, ctime(&mtime));
        #endif
    }

    return res;
}

int fs_rm(const char *fileName) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if (isValidFileName(fileName) == false) {
        printf("Error: Invalid filename %s.\n", fileName);
        return FS_FAILURE;
    }

    int res = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (res == FS_NOT_FOUND) {
        printf("Error: Fail to remove %s. No such file.\n", fileName);
        return FS_FAILURE;
    }

    res = deleteFileDirectoryByName(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (res == FS_FAILURE) {
        printf("Error: Fail to delete file directory %s from FAT\n", fileName);
    }

    return res;
}

int fs_mv(const char *src, const char *dst) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if (isValidFileName(src) == false) {
        printf("Error: Invalid filename %s.\n", src);
        return FS_FAILURE;
    }

    if (isValidFileName(dst) == false) {
        printf("Error: Invalid filename %s.\n", dst);
        return FS_FAILURE;
    }

    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, src);
    if (directoryEntryOffset == FS_NOT_FOUND) {
        printf("Error: Fail to move %s. No such file.\n", src);
        return FS_FAILURE;
    }

    if (strcmp(src, dst) == 0) {
        return FS_SUCCESS;
    }

    /* Remove dst from FAT. */
    deleteFileDirectoryByName(fs_FATConfig, fs_FAT16InMemory, dst);

    char buffer[MAX_FILE_NAME_LENGTH];
    memset(buffer, '\0', MAX_FILE_NAME_LENGTH);
    strcpy(buffer, dst);

    int fd = open(fs_FATConfig->name, O_WRONLY);
    lseek(fd, directoryEntryOffset, SEEK_SET);
    write(fd, buffer, sizeof(char) * MAX_FILE_NAME_LENGTH);
    close(fd);

    #ifdef FS_DEBUG_INFO
    printf("File %s is moved to %s", src, dst);
    #endif

    return FS_SUCCESS;
}

int fs_cp(const char *src, const char *dst) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if (isValidFileName(src) == false) {
        printf("Error: Invalid filename %s.\n", src);
        return FS_FAILURE;
    }

    if (isValidFileName(dst) == false) {
        printf("Error: Invalid filename %s.\n", dst);
        return FS_FAILURE;
    }

    int srcDirectoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, src);
    if (srcDirectoryEntryOffset == FS_NOT_FOUND) {
        printf("Error: Fail to copy %s. No such file.\n", src);
        return FS_FAILURE;
    }

    if (strcmp(src, dst) == 0) {
        return FS_SUCCESS;
    }

    /* Remove dst from FAT. */
    deleteFileDirectoryByName(fs_FATConfig, fs_FAT16InMemory, dst);
    int dstDirectoryEntryOffset = fs_touch(dst);

    DirectoryEntry srcDir;
    readDirectoryEntry(fs_FATConfig, srcDirectoryEntryOffset, &srcDir);

    DirectoryEntry dstDir;
    readDirectoryEntry(fs_FATConfig, dstDirectoryEntryOffset, &dstDir);

    int byteToWrite = srcDir.size;

    if (byteToWrite != 0) {
        int srcStartBlock = srcDir.firstBlock;
        int dstStartBlock = findEmptyFAT16Entry(fs_FATConfig, fs_FAT16InMemory);
        if (dstStartBlock == FS_NOT_FOUND) {
            printf("Error: Fail to copy %s. No empty data block.\n", src);
            return FS_FAILURE;
        }

        char buffer[byteToWrite];
        fs_readFAT(srcStartBlock, 0, byteToWrite, buffer);
        int res = fs_writeFAT(dstStartBlock, 0, byteToWrite, buffer);

        dstDir.firstBlock = dstStartBlock;
        dstDir.size = res;
        dstDir.mtime = time(NULL);
        writeFileDirectory(fs_FATConfig, dstDirectoryEntryOffset, &dstDir);

        if (res < byteToWrite) {
            printf("Error: Fail to copy entire %s. No enough space.\n", src);
            return FS_FAILURE;
        }
    }

    #ifdef FS_DEBUG_INFO
    printf("File %s is copied to %s.\n", src, dst);
    #endif

    return FS_SUCCESS;
}

int fs_readFAT(int startBlock, int startBlockOffset, int size, char *buffer) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if ((startBlock < 2) || (startBlockOffset > fs_FATConfig->FATEntryNum - 1)) {
        printf("Error: Invalid block idx %d.\n", startBlock);
        return FS_FAILURE;
    }

    if ((startBlockOffset < 0) || (startBlockOffset > fs_FATConfig->blockSize)) {
        printf("Error: Invalid block offset %d.\n", startBlockOffset);
        return FS_FAILURE;
    }

    if ((size < 0) || (size > fs_FATConfig->dataRegionSize - fs_FATConfig->blockSize)) {
        printf("Error: Invalid read size %d.\n", size);
        return FS_FAILURE;
    }

    return readFAT(fs_FATConfig, fs_FAT16InMemory, startBlock, startBlockOffset, size, buffer);
}

int fs_writeFAT(int startBlock, int startBlockOffset, int size, const char *buffer) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if ((startBlock < 2) || (startBlockOffset > fs_FATConfig->FATEntryNum - 1)) {
        printf("Error: Invalid block idx %d.\n", startBlock);
        return FS_FAILURE;
    }

    if ((startBlockOffset < 0) || (startBlockOffset > fs_FATConfig->blockSize)) {
        printf("Error: Invalid block offset %d.\n", startBlockOffset);
        return FS_FAILURE;
    }

    if ((size < 0) || (size > fs_FATConfig->dataRegionSize - fs_FATConfig->blockSize)) {
        printf("Error: Invalid write size %d.\n", size);
        return FS_FAILURE;
    }

    return writeFAT(fs_FATConfig, fs_FAT16InMemory, startBlock, startBlockOffset, size, buffer);
}

int fs_chmod(char *fileName, uint8_t perm) {
    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    DirectoryEntry dir;
    readDirectoryEntry(fs_FATConfig, directoryEntryOffset, &dir);  
    dir.perm = perm;
    dir.mtime = time(NULL);
    writeFileDirectory(fs_FATConfig, directoryEntryOffset, &dir);
    return 0;
}