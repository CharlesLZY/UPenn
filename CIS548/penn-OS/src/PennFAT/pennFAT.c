/**
 * @file pennFAT.c
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "pennFAT.h"

bool pf_isMounted() {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        return false;
    } else {
        return true;
    }
}

int pf_readFile(const char *fileName, int size, char *buffer) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (directoryEntryOffset == FS_NOT_FOUND) {
        printf("Error: Fail to read %s. No such file.\n", fileName);
        return -1;
    }

    DirectoryEntry dir;
    readDirectoryEntry(fs_FATConfig, directoryEntryOffset, &dir);
    if (dir.firstBlock != NO_FIRST_BLOCK) {
        int startBlock = dir.firstBlock;
        int startBlockOffset = 0;
        if (size > dir.size) {
            size = dir.size;
        }
        return fs_readFAT(startBlock, startBlockOffset, size, buffer);
    }

    return 0;
}

int pf_writeFile(const char *fileName, int size, const char *buffer, PF_WRITEMODE mode) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);

    DirectoryEntry dir;
    int startBlock;
    int startBlockOffset;

    if (directoryEntryOffset == FS_NOT_FOUND) {
        directoryEntryOffset = fs_touch(fileName); // create a new file directory
    } else {
        if (mode == PF_OVERWRITE) {
            deleteFileDirectoryByName(fs_FATConfig, fs_FAT16InMemory, fileName); // delete the original file directory
        }
        directoryEntryOffset = fs_touch(fileName);
    }
    
    readDirectoryEntry(fs_FATConfig, directoryEntryOffset, &dir);
    if (dir.firstBlock == NO_FIRST_BLOCK) {
        startBlock = findEmptyFAT16Entry(fs_FATConfig, fs_FAT16InMemory);
        if (startBlock == FS_NOT_FOUND) {
            printf("Error: The filesystem is full. No empty data block.\n");
            return -1;
        }
        startBlockOffset = 0;
        fs_FAT16InMemory[startBlock] = NO_SUCC_FAT_ENTRY;
        dir.firstBlock = (uint16_t) startBlock;
    } else {
        int fileEndOffset = traceFileEnd(fs_FATConfig, fs_FAT16InMemory, fileName);
        startBlockOffset = fileEndOffset % fs_FATConfig->blockSize;
        startBlock = (fileEndOffset - fs_FATConfig->FATRegionSize) / fs_FATConfig->blockSize + 1;
    }
    
    int res = fs_writeFAT(startBlock, startBlockOffset, size, buffer);
    dir.mtime = time(NULL);
    dir.size += res;
    return writeFileDirectory(fs_FATConfig, directoryEntryOffset, &dir);
}

int pf_mkfs(const char *fsName, int BLOCKS_IN_FAT, int BLOCK_SIZE_CONFIG) {
    if ((BLOCKS_IN_FAT < 1) || (BLOCKS_IN_FAT > 32)) {
        printf("Error: Invalid BLOCKS_IN_FAT.\n");
        return FS_FAILURE;
    }

    if ((BLOCK_SIZE_CONFIG < 0) || (BLOCK_SIZE_CONFIG > 4)) {
        printf("Error: Invalid BLOCK_SIZE_CONFIG.\n");
        return FS_FAILURE;
    }

    return fs_mkfs(fsName, BLOCK_SIZE_CONFIG, BLOCKS_IN_FAT);
}

int pf_mount(const char *fsName) {
    if (pf_isMounted()) {
        fs_unmount();
    }

    return fs_mount(fsName);
}

int pf_umount() {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    return fs_unmount();
}

int pf_touch(const char *fileName) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    return fs_touch(fileName);
}

int pf_rm(const char *fileName) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    return fs_rm(fileName);
}

int pf_mv(const char *src, const char *dst) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    return fs_mv(src, dst);
}

int pf_ls() {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    #ifdef FS_DEBUG_INFO
    printf("List all file directories:\n");
    #endif

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

    return 0;
}

int pf_chmod(const char *fileName, uint8_t perm) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (directoryEntryOffset == FS_NOT_FOUND) {
        printf("Error: Fail to chmod %s. No such file.\n", fileName);
        return -1;
    }

    DirectoryEntry dir;
    readDirectoryEntry(fs_FATConfig, directoryEntryOffset, &dir);  
    dir.perm = perm;
    dir.mtime = time(NULL);
    writeFileDirectory(fs_FATConfig, directoryEntryOffset, &dir);

    return 0;
}

int pf_catFiles(char **fileNames, int fileNum, int *size, char *buffer) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }
    
    int dirEntryOffsets[fileNum];
    for (int i = 0; i < fileNum; i++) {
        if (isValidFileName(fileNames[i]) == false) {
            printf("Error: Invalid filename %s.\n", fileNames[i]);
            return -1;
        }

        int res = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileNames[i]);
        if (res == FS_NOT_FOUND) {
            printf("Error: Fail to cat %s. No such file.\n", fileNames[i]);
            return -1;
        }

        dirEntryOffsets[i] = res;
    }

    int byteRead = 0;
    DirectoryEntry dir;
    for (int i = 0; i < fileNum; i++) {
        readDirectoryEntry(fs_FATConfig, dirEntryOffsets[i], &dir); 
        if (byteRead + dir.size > PF_MAX_BUFFER_SIZE) {
            printf("Error: Unable to cat all files. Out of PF_MAX_BUFFER_SIZE.");
            *size = byteRead;
            return -1;

        } else {
            pf_readFile(dir.name, dir.size, buffer);
            byteRead += dir.size;
            buffer += dir.size;
        }
    }

    *size = byteRead;
    #ifdef FS_DEBUG_INFO
    printf("pf_catFile() read %d bytes.\n", byteRead);
    #endif

    return 0;
}



void SIGINTHandler(int sig) {
    /* Ignore Ctrl+C */
    printf("\n");
}

char *readInput(char *inputBuffer) {
    int numBytes = read(STDIN_FILENO, inputBuffer, MAX_LINE_LENGTH);
    if (numBytes == -1) {
        printf("Failed to read the user input.\n");
        return NULL;
    }

    if (numBytes == 0) { // read nothing but EOF (Ctrl + D at the beginning of the input line)
        return NULL;
    }

    if (inputBuffer[numBytes - 1] != '\n') { // input ended with EOF (Ctrl + D)
        char *userInput = malloc(1);
        userInput[0] = '\0';
        return userInput;
    } else {
        char *userInput = malloc(numBytes);
        for (int i = 0; i < numBytes; i++) {
            if (inputBuffer[i] == '\n') {
                userInput[i] = '\0';
            } 
            else {
                userInput[i] = inputBuffer[i];
            }
        }
        return userInput;
    }
}

int parseInput(char *userInput, char **argsBuffer, int *argNum) {
    if (userInput == NULL) {
        return EXIT_SHREDDER;
    }
    char *t = strtok(userInput, " \t\n"); // split the input by the whitespaces
    argsBuffer[0] = t;
    int n;
    for (n = 0; t != NULL; n++) {
        t = strtok(NULL, " \t\n");
        if (n >= MAX_ARGS_NUM) {
            break;
        }
        argsBuffer[n+1] = t;
    }
    *argNum = n;
    if (n == 0) {
        return EMPTY_LINE;
    }
    return EXECUTE_COMMAND;
}

// /* 
int main(int argc, char **argv) {
    printf("PennFAT is running... Ctrl+D to exit.\n");
    signal(SIGINT, SIGINTHandler);

    char inputBuffer[MAX_LINE_LENGTH];
    char *argsBuffer[MAX_ARGS_NUM] = {NULL};
    int argNum = 0;
    while (1) {
        write(STDERR_FILENO, "pennfat# ", 9);
        char *userInput = readInput(inputBuffer);
        int parseResult = parseInput(userInput, argsBuffer, &argNum);
        if (parseResult == EXIT_SHREDDER) {
            break;
        }
        else if (parseResult == EMPTY_LINE) {
            printf("\n");
        }
        else {
            if (strcmp(argsBuffer[0], "mkfs") == 0) {
                if (argNum != 4) {
                    printf("Error: Invalid Command\n");
                } else {
                    int BLOCKS_IN_FAT = atoi(argsBuffer[2]);
                    int BLOCK_SIZE_CONFIG = atoi(argsBuffer[3]);
                    pf_mkfs(argsBuffer[1], BLOCKS_IN_FAT, BLOCK_SIZE_CONFIG);
                }
            } else if (strcmp(argsBuffer[0], "mount") == 0) {
                if (argNum != 2) {
                    printf("Error: Invalid Command\n");
                } else {
                    pf_mount(argsBuffer[1]);
                }
            } else if (strcmp(argsBuffer[0], "umount") == 0) {
                if (argNum != 1) {
                    printf("Error: Invalid Command\n");
                } else {
                    pf_umount();
                }
            } else if (strcmp(argsBuffer[0], "touch") == 0) {
                if ((argNum < 2) || (argNum > PF_MAX_FILE_NUM + 1)) {
                    printf("Error: Invalid Command\n");
                } else {
                    int flag = 0;
                    for (int i = 1; i < argNum; i++) {
                        if (isValidFileName(argsBuffer[i]) == false) {
                            printf("Error: Invalid File Name %s\n", argsBuffer[i]);
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 0) {
                        for (int i = 1; i < argNum; i++) {
                            pf_touch(argsBuffer[i]);
                        }
                    }
                }
            } else if (strcmp(argsBuffer[0], "mv") == 0) {
                if (argNum != 3 ) {
                    printf("Error: Invalid Command\n");
                } else {
                    if ((isValidFileName(argsBuffer[1]) == false) || (isValidFileName(argsBuffer[2]) == false)) {
                        printf("Error: Invalid File Name\n");
                    } else {
                        pf_mv(argsBuffer[1], argsBuffer[2]);
                    }
                }
            } else if (strcmp(argsBuffer[0], "rm") == 0) {
                if (argNum - 1 > PF_MAX_FILE_NUM ) {
                    printf("Error: Invalid Command\n");
                } else {
                    int invalid = 0;
                    for (int i = 1; i < argNum; i++) {
                        if (isValidFileName(argsBuffer[i]) == false) {
                            printf("Error: Invalid File Name %s\n", argsBuffer[i]);
                            invalid = 1;
                            break;
                        }
                    }
                    if (invalid != 1) {
                        for (int i = 1; i < argNum; i++) {
                            pf_rm(argsBuffer[i]);
                        }
                    }
                }
            } else if (strcmp(argsBuffer[0], "ls") == 0) {
                if (argNum != 1) {
                    printf("Error: Invalid Command\n");
                } else {
                    pf_ls();
                }
            } else if (strcmp(argsBuffer[0], "chmod") == 0) {
                if (argNum != 3) {
                    printf("Error: Invalid Command\n");
                } else {
                    int perm = atoi(argsBuffer[1]);
                    if ((perm < 0) || (perm > 7) || (perm == 1) || (perm == 3)) {
                        printf("Error: Invalid File Permission\n");
                    }
                    pf_chmod(argsBuffer[2], perm);
                }
            } else if (strcmp(argsBuffer[0], "cat") == 0) {
                if (argNum == 1) {
                    printf("Error: Invalid Command\n");
                    continue;
                }
                int invalid = 0;
                int flagLocation = -1;
                PF_WRITEMODE mode = PF_STDOUT;
                for (int i = 1; i < argNum; i++) {
                    if (strcmp(argsBuffer[i], "-w") == 0) {
                        if (flagLocation != -1) {
                            invalid = 1;
                        }
                        flagLocation = i;
                        mode = PF_OVERWRITE;
                    } else if (strcmp(argsBuffer[i], "-a") == 0) {
                        if (flagLocation != -1) {
                            invalid = 1;
                        }
                        flagLocation = i;
                        mode = PF_APPEND;
                    }
                }

                if (invalid == 1) { // multiple flags
                    printf("Error: Invalid Command\n");
                    continue;
                }

                if (flagLocation == -1) { // no flag
                    if (argNum - 1 > PF_MAX_FILE_NUM) {
                        printf("Error: Too Many Files.\n");
                        continue;
                    }

                    char buffer[PF_MAX_BUFFER_SIZE];
                    memset(buffer, 0, PF_MAX_BUFFER_SIZE);
                    int size = 0;
                    char *fileNames[PF_MAX_FILE_NUM];
                    for (int i = 1; i < argNum; i++) {
                        if (isValidFileName(argsBuffer[i]) == false) {
                            printf("Error: Invalid File Name\n");
                            invalid = 1;
                            break;
                        }
                        fileNames[i-1] = argsBuffer[i];
                    }

                    if (invalid == 1) {
                        continue;
                    }

                    int res = pf_catFiles(fileNames, argNum-1, &size, buffer);
                    if (res == 0) {
                        printf("%s\n", buffer);
                    }
                } else {
                    if (isValidFileName(argsBuffer[argNum - 1]) == false) {
                        printf("Error: Invalid File Name\n");
                        continue;
                    }

                    if (flagLocation != argNum - 2) { // multiple output file after the flag
                        printf("Error: Invalid Command\n");
                        continue;
                    }
                    if (flagLocation != 1) { // cat files
                       if (argNum - 3 > PF_MAX_FILE_NUM) {
                            printf("Error: Too Many Files.\n");
                            continue;
                        }

                        char buffer[PF_MAX_BUFFER_SIZE];
                        memset(buffer, 0 , PF_MAX_BUFFER_SIZE);

                        int size = 0;
                        char *fileNames[PF_MAX_FILE_NUM];
                        for (int i = 1; i < argNum - 2; i++) {
                            if (isValidFileName(argsBuffer[i]) == false) {
                                printf("Error: Invalid File Name\n");
                                invalid = 1;
                                break;
                            }
                            fileNames[i-1] = argsBuffer[i];
                        }

                        if (invalid == 1) {
                            continue;
                        }

                        int res = pf_catFiles(fileNames, argNum-3, &size, buffer);
                        if (res == 0) {
                            pf_writeFile(argsBuffer[argNum-1], size, buffer, mode);
                        }

                    } else { // read from terminal 
                        char buffer[PF_MAX_BUFFER_SIZE];
                        memset(buffer, 0 , PF_MAX_BUFFER_SIZE);
                        int size = read(STDIN_FILENO, buffer, PF_MAX_BUFFER_SIZE);
                        pf_writeFile(argsBuffer[argNum-1], size, buffer, mode);
                    }
                }
            } else if (strcmp(argsBuffer[0], "cp") == 0) {
                if (pf_isMounted() == false) {
                    printf("Error: No mounted file system.\n");
                    continue;
                }
                if ((argNum != 3) && (argNum != 4)) {
                    printf("Error: Invalid Command\n");
                    continue;
                }
                int invalid = 0;
                int flagLocation = -1;
                for (int i = 1; i < argNum; i++) {
                    if (strcmp(argsBuffer[i], "-h") == 0) {
                        if (flagLocation != -1) {
                            invalid = 1;
                        }
                        flagLocation = i;
                    }
                }

                if (invalid == 1) { // multiple flags
                    printf("Error: Invalid Command\n");
                    continue;
                }

                if (flagLocation == -1) { // no flag
                    if (argNum == 4) {
                        printf("Error: Invalid Command\n");
                        continue;
                    }
                    if ((isValidFileName(argsBuffer[1]) == false) || (isValidFileName(argsBuffer[2]) == false)) {
                        printf("Error: Invalid File Name\n");
                    } else {
                        fs_cp(argsBuffer[1], argsBuffer[2]);
                    }
                } else {
                    if (argNum == 3) {
                        printf("Error: Invalid Command\n");
                        continue;
                    }
                    if (flagLocation == 1) {
                        char *dst = argsBuffer[3];
                        if (isValidFileName(dst) == false) {
                            printf("Error: Invalid File Name %s\n", dst);
                            continue;
                        }
                        char *hostSrc = argsBuffer[2];
                        int srcFd = open(hostSrc, O_RDONLY);
                        if (srcFd == -1) {
                            printf("Error: Fail to open %s on host OS\n", hostSrc);
                            continue;
                        }

                        char buffer[PF_MAX_BUFFER_SIZE];
                        memset(buffer, 0 , PF_MAX_BUFFER_SIZE);
                        int size = read(srcFd, buffer, PF_MAX_BUFFER_SIZE);
                        close(srcFd);
                        pf_writeFile(dst, size, buffer, PF_OVERWRITE);

                    } else if (flagLocation == 2) {
                        char *src = argsBuffer[1];
                        if (isValidFileName(src) == false) {
                            printf("Error: Invalid File Name %s\n", src);
                            continue;
                        }
                        char *hostDst = argsBuffer[3];
                        int dstFd = open(hostDst, O_WRONLY | O_CREAT | O_TRUNC);
                        if (dstFd == -1) {
                            printf("Error: Fail to open %s on host OS\n", hostDst);
                            continue;
                        }
                        char buffer[PF_MAX_BUFFER_SIZE];
                        memset(buffer, 0 , PF_MAX_BUFFER_SIZE);
                        int size = pf_readFile(src, PF_MAX_BUFFER_SIZE, buffer);
                        write(dstFd, buffer, size);
                        close(dstFd);

                    } else {
                        printf("Error: Invalid Command\n");
                        continue;
                    }
                }

            } else {
                printf("Error: Invalid Command\n");
            }


        }
        free(userInput);
    }

    if (pf_isMounted()) {
        pf_umount();
    }

    return 0;
}

// */