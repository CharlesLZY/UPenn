/**
 * @file FAT.c
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "FAT.h"

/* LSB is block size & MSB is block num of FAT region */
FATConfig *createFATConfig(const char *name, uint16_t LSB, uint16_t MSB) {
    FATConfig *newConfig = malloc(sizeof(FATConfig));

    strcpy(newConfig->name, name);
    newConfig->LSB = LSB;
    newConfig->MSB = MSB;
    newConfig->blockSize = MIN_BLOCK_SIZE << LSB;
    newConfig->FATRegionBlockNum = MSB;
    newConfig->FATRegionSize = newConfig->blockSize * newConfig->FATRegionBlockNum;
    newConfig->FATEntryNum = newConfig->FATRegionSize / FAT_ENTRY_SIZE;
    newConfig->dataRegionSize = newConfig->blockSize * (newConfig->FATEntryNum - 1);

    int pageSize = (int) sysconf(_SC_PAGESIZE);
    newConfig->FATSizeInMemory = newConfig->FATRegionSize + pageSize - newConfig->FATRegionSize % pageSize; // align to page size
    return newConfig;
}

uint16_t *createFAT16InMemory(FATConfig *config) {
    uint16_t *newFAT16 = malloc(config->FATSizeInMemory); // uint16_t is of size 2-byte, therefore, it can be considered as the entry of PennFAT which is based on FAT16
    memset(newFAT16, EMPTY_FAT_ENTRY, config->FATSizeInMemory); // initialize FAT entries to 0x0000

    newFAT16[0] = config->LSB + (config->MSB << 8); // The first entry of the FAT will specify the config.
    newFAT16[1] = NO_SUCC_FAT_ENTRY; // The first block of data region will be used for root directory.
    /* The remaining entries are set to 0x0000. */

    return newFAT16;
}

/* Because of endianness, hexdump may display FAT differently from the one in memory.*/
int createFATOnDisk(FATConfig *config) {
    int fd = open(config->name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("Open Error: Fail to open %s\n", config->name);
        return FS_FAILURE;
    }

    /* write the FAT region to file */
    uint16_t *FAT = createFAT16InMemory(config);
    int res = write(fd, FAT, config->FATRegionSize);
    free(FAT);

    if (res < config->FATRegionSize) {
        printf("Write Error: Fail to write FAT region on the disk.\n");
        return FS_FAILURE;
    }

    /* claim space for the data region */
    char writeBuffer[config->blockSize];

    /* Initialize the root directory block. */
    memset(writeBuffer, DIRECTORY_END[0], config->blockSize);
    res = write(fd, writeBuffer, config->blockSize);
    if (res < config->blockSize) {
        printf("Write Error: Fail to write data block on the disk.\n");
        return FS_FAILURE;
    }

    /* Initialize the remaining data block. */
    memset(writeBuffer, '\0', config->blockSize);
    for (int i = 2; i < config->FATEntryNum; i++) {
        res = write(fd, writeBuffer, config->blockSize);
        if (res < config->blockSize) {
            printf("Write Error: Fail to write data block on the disk.\n");
            return FS_FAILURE;
        }
    }

    close(fd);
    return FS_SUCCESS;
}

int findEmptyFAT16Entry(FATConfig *config, uint16_t *FAT16) {
    for (int i = 1; i < config->FATEntryNum; i++) { // the first and second entry of FAT are config and root directory
        if (FAT16[i] == EMPTY_FAT_ENTRY) {
            return i;
        }
    }

    return FS_NOT_FOUND;
}

DirectoryEntry *createDirectoryEntry(const char *name, uint32_t size, uint16_t firstBlock, uint8_t type, uint8_t perm) {
    DirectoryEntry *newEntry = malloc(DIRECTORY_ENTRY_SIZE);
    memset(newEntry, 0, DIRECTORY_ENTRY_SIZE); // initialize the memory to 0 for the reserved space

    strcpy(newEntry->name, name);
    newEntry->size = size;
    newEntry->firstBlock = firstBlock;
    newEntry->type = type;
    newEntry->perm = perm;
    newEntry->mtime = time(NULL);
    /* the reserved space is set to 0 */

    return newEntry;
}

int createFileDirectoryOnDisk(FATConfig *config, uint16_t *FAT16, const char *fileName, uint8_t fileType, uint8_t filePerm) {
    int fd = open(config->name, O_RDWR);
    int dataRegionOffset = config->FATRegionSize;
    int blockSize = config->blockSize;
    
    uint16_t curBlock = 1; // root directory is the first entry of the data region

    char marker; // name[0] will serve as a special marker to indicate the state of the directory entry
    while (true) { // Traverse all directory blocks.
        for (int i = 0; i < blockSize / DIRECTORY_ENTRY_SIZE; i++) { // Check all directory entries.
            int offset = dataRegionOffset + (curBlock - 1) * blockSize + i * DIRECTORY_ENTRY_SIZE;
            lseek(fd, offset, SEEK_SET);
            read(fd, &marker, sizeof(char));
            if (marker == DIRECTORY_END[0] || marker == DELETED_DIRECTORY[0]) {
                /* Write directory entry. */
                DirectoryEntry *newEntry = createDirectoryEntry(fileName, 0, NO_FIRST_BLOCK, fileType, filePerm);
                lseek(fd, offset, SEEK_SET);
                write(fd, newEntry, DIRECTORY_ENTRY_SIZE);

                free(newEntry);
                close(fd);
                return offset;
            }   
        }

        if (FAT16[curBlock] == NO_SUCC_FAT_ENTRY) { // There is no available entry in the current directory block and there is not more directory block.

            int newDirectoryBlock = findEmptyFAT16Entry(config, FAT16);

            if (newDirectoryBlock == FS_NOT_FOUND) {
                printf("Error: No empty data block. Fail to create a new directory block.\n");
                close(fd);
                return FS_FAILURE;
            }

            #ifdef FS_DEBUG_INFO
            printf("Claim a new directory block with idx = %d\n", newDirectoryBlock);
            #endif

            FAT16[curBlock] = newDirectoryBlock;

            /* Initialize the new directory block. */
            FAT16[newDirectoryBlock] = NO_SUCC_FAT_ENTRY;

            int offset = dataRegionOffset + (newDirectoryBlock - 1) * blockSize;
            lseek(fd, offset, SEEK_SET);

            char writeBuffer[blockSize];
            memset(writeBuffer, DIRECTORY_END[0], blockSize);
            write(fd, writeBuffer, blockSize);
        }

        curBlock = FAT16[curBlock]; // Go to the next directory block.
    }

    printf("Error: Something unexpected happened in createFileDirectoryOnDisk().\n");
    close(fd);
    return FS_FAILURE;

}

int findFileDirectory(FATConfig *config, uint16_t *FAT16, const char *fileName) {
    int fd = open(config->name, O_RDONLY);
    int dataRegionOffset = config->FATRegionSize;
    int blockSize = config->blockSize;
    
    uint16_t curBlock = 1; // root directory is the first entry of the data region
    char *readBuffer = malloc(MAX_FILE_NAME_LENGTH);

    while (true) { // Traverse all directory blocks.
        
        for (int i = 0; i < blockSize / DIRECTORY_ENTRY_SIZE; i++) { // Check all directory entries.
            int offset = dataRegionOffset + (curBlock - 1) * blockSize + i * DIRECTORY_ENTRY_SIZE;
            lseek(fd, offset, SEEK_SET);
            read(fd, readBuffer, sizeof(char) * MAX_FILE_NAME_LENGTH);
            if (strcmp(readBuffer, fileName) == 0) {

                #ifdef FS_DEBUG_INFO
                printf("File directory of %s is found.\n", fileName);
                #endif

                free(readBuffer);
                close(fd);
                return offset;
            }
        }
        

        if (FAT16[curBlock] == NO_SUCC_FAT_ENTRY) { // There is no available entry in the current directory block and there is not another directory block.
            break;
        } else {
            curBlock = FAT16[curBlock];
            if (curBlock >= config->FATEntryNum) {
                printf("Error: Invalid block idx %d", curBlock);
                return FS_FAILURE;
            }
        }

    }

    #ifdef FS_DEBUG_INFO
    printf("File directory of %s does not exist.\n", fileName);
    #endif

    free(readBuffer);
    close(fd);
    return FS_NOT_FOUND;
}

int readDirectoryEntry(FATConfig *config, int offset, DirectoryEntry *dir) {
    int fd = open(config->name, O_RDONLY);
    lseek(fd, offset, SEEK_SET);

    int res = read(fd, dir, DIRECTORY_ENTRY_SIZE);
    if (res == -1) {
        perror("Read Error: Unexpected Behavior");
        close(fd);
        return FS_FAILURE;
    } else if (res < DIRECTORY_ENTRY_SIZE) {
        perror("Read Error: Unable to read whole directory entry.");
        close(fd);
        return FS_FAILURE;
    }

    close(fd);
    return FS_SUCCESS;
}

int writeFileDirectory(FATConfig *config, int offset, DirectoryEntry *dir) {
    int fd = open(config->name, O_WRONLY);
    lseek(fd, offset, SEEK_SET);

    int res = write(fd, dir, DIRECTORY_ENTRY_SIZE);
    if (res == -1) {
        perror("Write Error: Unexpected Behavior");
        close(fd);
        return FS_FAILURE;
    } else if (res < DIRECTORY_ENTRY_SIZE) {
        perror("Write Error: Unable to write whole directory entry.");
        close(fd);
        return FS_FAILURE;
    }

    close(fd);
    return FS_SUCCESS;
}

int deleteFileDirectory(FATConfig *config, uint16_t *FAT16, int offset) {
    DirectoryEntry dir;
    int res = readDirectoryEntry(config, offset, &dir);
    if (res == FS_FAILURE) {
        printf("Error: Fail to read directory entry at offset %d.\n", offset);
        return FS_FAILURE;
    }

    /* Mark the directory entry as deleted. */
    int fd = open(config->name, O_WRONLY);
    lseek(fd, offset, SEEK_SET);
    write(fd, DELETED_DIRECTORY, sizeof(char));
    close(fd);

    /* Release all blocks occupied by the file in FAT. */
    uint16_t curBlock = dir.firstBlock;

    if (curBlock == NO_FIRST_BLOCK) { // It is an empty file.
        #ifdef FS_DEBUG_INFO
        printf("No block to release.\n");
        #endif

        return FS_SUCCESS;
    }

    while (curBlock != NO_SUCC_FAT_ENTRY) {
        if (curBlock >= config->FATEntryNum) {
            printf("Error: Invalid block idx %d\n", curBlock);
            return FS_FAILURE;
        }

        uint16_t nextBlock = FAT16[curBlock];
        FAT16[curBlock] = EMPTY_FAT_ENTRY;
        /* Here we perform the lazy deletion. The data block will not be flushed. */

        #ifdef FS_DEBUG_INFO
        printf("Block %d is released.\n", curBlock);
        #endif

        curBlock = nextBlock;
    }
        
    return FS_SUCCESS;
}

int deleteFileDirectoryByName(FATConfig *config, uint16_t *FAT16, const char *fileName) {
    int offset = findFileDirectory(config, FAT16, fileName);
    if (offset == -1) {
        #ifdef FS_DEBUG_INFO
        printf("Warning: No such file directory %s to delete.\n", fileName);
        #endif
        return FS_SUCCESS;
    }

    int res = deleteFileDirectory(config, FAT16, offset);

    if (res == FS_SUCCESS) {
        #ifdef FS_DEBUG_INFO
        printf("File %s's directory entry is deleted from FAT.\n", fileName);
        #endif
    }

    return res;
}

bool isDirectoryEntryToDelete(FATConfig *config, int directoryEntryOffset) {
    int fd = open(config->name, O_RDWR);
    char marker; // name[0] will serve as a special marker to indicate the state of the directory entry
    lseek(fd, directoryEntryOffset, SEEK_SET);
    read(fd, &marker, sizeof(char));
    return marker == DELETED_DIRECTORY_IN_USE[0];
}

int readFAT(FATConfig *config, uint16_t *FAT16, int startBlock, int startBlockOffset, int size, char *buffer) {
    int fd = open(config->name, O_RDONLY);
    int dataRegionOffset = config->FATRegionSize;
    int blockSize = config->blockSize;

    int byteToRead = size;
    int curBlock = startBlock;
    int curBlockOffset = startBlockOffset;
    while (byteToRead > 0) {
        if (FAT16[curBlock] == EMPTY_FAT_ENTRY) {
            printf("Error: Invalid read. The block %d is unused.\n", curBlock);
            return FS_FAILURE;
        }

        int offset = dataRegionOffset + (curBlock - 1) * blockSize + curBlockOffset;
        lseek(fd, offset, SEEK_SET);
        int curByteToRead = byteToRead < blockSize - curBlockOffset ? byteToRead : blockSize - curBlockOffset;
        
        int res = read(fd, buffer, curByteToRead);
        if (res == -1) {
            perror("Read Error: Unexpected Behavior");
            close(fd);
            return FS_FAILURE;
        } else if (res < curByteToRead) {
            perror("Read Error: Unable to read all bytes.");
            close(fd);
            return res + size - byteToRead;
        }

        byteToRead -= curByteToRead;

        #ifdef FS_DEBUG_INFO
        printf("Read %d bytes from block %d with offset %d.\n", curByteToRead, curBlock, curBlockOffset);
        #endif

        if (byteToRead > 0) { // There are still some bytes to read.
            if (FAT16[curBlock] == NO_SUCC_FAT_ENTRY) {
                printf("Error: %d bytes to read. But file ends at block %d.\n", curBlock, byteToRead);
                return size - byteToRead;
            }

            curBlock = FAT16[curBlock];
            curBlockOffset = 0;

            buffer += curByteToRead;
        }
    }

    #ifdef FS_DEBUG_INFO
    printf("Successfully read %d bytes.\n", size);
    #endif

    close(fd);
    return size;
}

int writeFAT(FATConfig *config, uint16_t *FAT16, int startBlock, int startBlockOffset, int size, const char *buffer) {
    int fd = open(config->name, O_WRONLY);
    int dataRegionOffset = config->FATRegionSize;
    int blockSize = config->blockSize;

    int byteToWrite = size;
    int curBlock = startBlock;
    int curBlockOffset = startBlockOffset;
    while (byteToWrite > 0) {
        /* Write to the current block. */
        if (FAT16[curBlock] == EMPTY_FAT_ENTRY) {
            FAT16[curBlock] = NO_SUCC_FAT_ENTRY; // this block is used now
        }

        int offset = dataRegionOffset + (curBlock - 1) * blockSize + curBlockOffset;
        lseek(fd, offset, SEEK_SET);
        int curByteToWrite = byteToWrite < blockSize - curBlockOffset ? byteToWrite : blockSize - curBlockOffset;
        
        int res = write(fd, buffer, curByteToWrite);
        if (res == -1) {
            perror("Write Error: Unexpected Behavior\n");
            close(fd);
            return FS_FAILURE;
        } else if (res < curByteToWrite) {
            perror("Write Error: Unable to write all bytes.\n");
            close(fd);
            return res + size - byteToWrite;
        }
    
        byteToWrite -= curByteToWrite;
        
        #ifdef FS_DEBUG_INFO
        printf("Write %d bytes to block %d with offset %d.\n", curByteToWrite, curBlock, curBlockOffset);
        #endif

        if (byteToWrite > 0) { // There are still some bytes to write.
            /* Find a new empty block to write. */
            int nextBlock = findEmptyFAT16Entry(config, FAT16);

            if (nextBlock == FS_NOT_FOUND) {
                printf("Error: No emtpy data block. Fail to write remaining %d bytes.\n", byteToWrite);
                close(fd);
                return size - byteToWrite;
            }

            #ifdef FS_DEBUG_INFO
            printf("Claim a new data block with idx = %d.\n", nextBlock);
            #endif

            FAT16[curBlock] = nextBlock;
            curBlock = nextBlock;
            curBlockOffset = 0;

            buffer += curByteToWrite;
        }
    }

    #ifdef FS_DEBUG_INFO
    printf("Successfully write %d bytes.\n", size);
    #endif

    close(fd);
    return size;
}

int traceFileEnd(FATConfig *config, uint16_t *FAT16, const char *fileName) {
    int directoryEntryOffset = findFileDirectory(config, FAT16, fileName);
    if (directoryEntryOffset == -1) {

        #ifdef FS_DEBUG_INFO
        printf("Error: No such file directory %s to trace end.\n", fileName);
        #endif

        return FS_NOT_FOUND;
    }

    DirectoryEntry dir;
    int res = readDirectoryEntry(config, directoryEntryOffset, &dir);
    if (res == FS_FAILURE) {
        printf("Error: Fail to read directory entry at offset %d.\n", directoryEntryOffset);
        return FS_FAILURE;
    }

    int offset = dir.size % config->blockSize; // file end offset of an empty file will be 0

    uint16_t curBlock = dir.firstBlock;
    if (curBlock == NO_FIRST_BLOCK) { // It is an empty file.

        #ifdef FS_DEBUG_INFO
        printf("Warning: File %s is an empty file. The end offset is undecided.\n", fileName);
        #endif

        return 0;
    }

    while (FAT16[curBlock] != NO_SUCC_FAT_ENTRY) {
        if (curBlock >= config->FATEntryNum) {
            printf("Error: Invalid block idx %d.\n", curBlock);
            return FS_FAILURE;
        }
        curBlock = FAT16[curBlock];
    }

    offset += config->FATRegionSize + (curBlock - 1) * config->blockSize;

    #ifdef FS_DEBUG_INFO
    printf("The end offset of file %s is %d.\n", fileName, offset);
    #endif

    return offset;
}

int traceBytesFromBeginning(FATConfig *config, uint16_t *FAT16, int directoryEntryOffset, int fileOffset) {
    DirectoryEntry dir;
    int res = readDirectoryEntry(config, directoryEntryOffset, &dir);
    if (res == FS_FAILURE) {
        printf("Error: Fail to read directory entry at offset %d.\n", directoryEntryOffset);
        return FS_FAILURE;
    }

    if (dir.firstBlock == NO_FIRST_BLOCK) {
        printf("Error: Fail to trace byte for an empty file.\n");
        return FS_FAILURE;
    }

    int dataRegionOffset = config->FATRegionSize;
    int blockSize = config->blockSize;
    int targetBlock = (fileOffset - dataRegionOffset) / blockSize + 1;
    int curBlock = dir.firstBlock;

    int blockNum = 0;
    while (curBlock != targetBlock) {
        if (curBlock == NO_SUCC_FAT_ENTRY) {
            printf("Error: Fail to trace byte. Exceed file's data blocks.\n");
            return FS_FAILURE;
        }
        curBlock = FAT16[curBlock];
        blockNum += 1;
    }

    return blockNum * blockSize + fileOffset % blockSize;
}

int traceBytesToEnd(FATConfig *config, uint16_t *FAT16, int directoryEntryOffset, int fileOffset) {
    int bytesFromBeginning = traceBytesFromBeginning(config, FAT16, directoryEntryOffset, fileOffset);
    if (bytesFromBeginning == FS_FAILURE) {
        printf("Error: Fail to trace byte to End.\n");
        return FS_FAILURE;
    }

    DirectoryEntry dir;
    readDirectoryEntry(config, directoryEntryOffset, &dir);
    return dir.size - bytesFromBeginning;
}

int traceOffset(FATConfig *config, uint16_t *FAT16, int directoryEntryOffset, int fileOffset, int n) {
    DirectoryEntry dir;
    int res = readDirectoryEntry(config, directoryEntryOffset, &dir);
    if (res == FS_FAILURE) {
        printf("Error: Fail to read directory entry at offset %d.\n", directoryEntryOffset);
        return FS_FAILURE;
    }

    if (dir.firstBlock == NO_FIRST_BLOCK) {
        printf("Error: Fail to trace offset for an empty file.\n");
        return FS_FAILURE;
    }

    int bytesFromBeginning = traceBytesFromBeginning(config, FAT16, directoryEntryOffset, fileOffset);
    if (bytesFromBeginning == FS_FAILURE) {
        printf("Error: Invalid file Offset. Exceed file's data blocks.\n");
        return FS_FAILURE;
    }

    if (n == 0) { // 0 byte to trace
        return fileOffset;
    }

    int dataRegionOffset = config->FATRegionSize;
    int blockSize = config->blockSize;

    int byteToTrace = n;
    int byteTraced = 0;
    int curBlock = fileOffset / blockSize;
    int curBlockOffset = fileOffset % blockSize;

    while (byteToTrace > 0) {
        /* Traverse the current block. */
        if (FAT16[curBlock] == EMPTY_FAT_ENTRY) {
            FAT16[curBlock] = NO_SUCC_FAT_ENTRY; // this block is used now
        }
        int curByteToTrace = byteToTrace < blockSize - curBlockOffset ? byteToTrace : blockSize - curBlockOffset;
        byteToTrace -= curByteToTrace;
        byteTraced += curByteToTrace;

        #ifdef FS_DEBUG_INFO
        printf("Trace %d bytes to block %d with offset %d.\n", curByteToTrace, curBlock, curBlockOffset);
        #endif

        if (byteToTrace > 0) { // There are still some bytes to trace.
            if (FAT16[curBlock] == NO_SUCC_FAT_ENTRY) {
                /* Find a new empty block and occupy it for the file. */
                int nextBlock = findEmptyFAT16Entry(config, FAT16);

                if (nextBlock == FS_NOT_FOUND) {
                    printf("Error: No emtpy data block. Fail to trace remaining %d bytes.\n", byteToTrace);
                    
                    if (bytesFromBeginning + byteTraced > dir.size) {
                        dir.size = bytesFromBeginning + byteTraced;
                        writeFileDirectory(config, directoryEntryOffset, &dir);
                    }

                    return curBlock * blockSize;
                }

                #ifdef FS_DEBUG_INFO
                printf("Claim a new data block with idx = %d.\n", nextBlock);
                #endif

                FAT16[curBlock] = nextBlock;
                curBlockOffset = 0;
            }
            curBlock = FAT16[curBlock];
        }
    }

    if (bytesFromBeginning + byteTraced > dir.size) {
        dir.size = bytesFromBeginning + byteTraced;
        writeFileDirectory(config, directoryEntryOffset, &dir);
    }

    int tailOffset;
    if (byteTraced > 0) {
        tailOffset = ((bytesFromBeginning + byteTraced) % blockSize == 0) ? blockSize : (bytesFromBeginning + byteTraced) % blockSize;
    } else {
        tailOffset = (bytesFromBeginning + byteTraced) % blockSize;
    }
    return dataRegionOffset + (curBlock - 1) * blockSize + tailOffset;
}