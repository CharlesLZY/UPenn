/**
 * @file FAT.h
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef FAT_H
#define FAT_H

#include "utils.h"

/* PennFAT is based on FAT16 */
#define MIN_BLOCK_SIZE 256
#define MAX_FAT_BLOCK_NUM 32
#define MAX_BLOCK_SCALE 4

#define FAT_ENTRY_SIZE 2
#define EMPTY_FAT_ENTRY 0x0000
#define NO_SUCC_FAT_ENTRY 0xFFFF

#define MAX_FILE_NAME_LENGTH 32

/**
 * @brief The LSB(rightmost under little endian) of the first entry of the FAT specifies the block size with the mapping as below:
{LSB : size in bytes} = {0:256; 1:512; 2:1024; 3:2048; 4:4096}
The MSB(leftmost under little endian) of the first entry of the FAT specifies the number of blocks that FAT region occupies.
The MSB should be ranged from 1-32 (numbers outside of this range will be considered an error).
FAT region size = block size * FAT region block number
FAT entry number = FAT region size / FAT entry size (2-byte in FAT16)
Data region size = block size * (FAT entry number - 1)
*/
typedef struct FATConfig {
    char name[MAX_FILE_NAME_LENGTH];

    uint16_t LSB;
    uint16_t MSB;

    int blockSize;
    int FATRegionBlockNum;
    int FATRegionSize;
    int FATEntryNum;
    int dataRegionSize;

    /*
    The FAT region will be mapped to the memory through mmap(). 
    The size of the mapping area must be a multiple of the memory page size.
    */
    int FATSizeInMemory;
} FATConfig;

/* PennFAT has a 64-byte fixed directory entry size (32 + 4 + 2 + 1 + 1 + 8 + 16 = 64 bytes)*/
#define DIRECTORY_ENTRY_SIZE 64

#define NO_FIRST_BLOCK 0x0000

#define DIRECTORY_END "0"
#define DELETED_DIRECTORY "1"
/*
Process A and Process B opened the same file. 
Process A unlinked the file but Process B was still using the file.
The file should be deleted after Process B closes the file.
*/
#define DELETED_DIRECTORY_IN_USE "2"

#define FILE_TYPE_UNKNOWN 0
#define FILE_TYPE_REGULAR 1
#define FILE_TYPE_DIRECTORY 2
#define FILE_TYPE_SYMBOLIC_LINK 4

#define FILE_PERM_NONE 0  // 000
#define FILE_PERM_WRITE 2 // 010
#define FILE_PERM_READ 4  // 100
#define FILE_PERM_READ_EXEC 5 // 101
#define FILE_PERM_READ_WRITE 6 // 110
#define FILE_PERM_READ_WRITE_EXEC 7 // 111

#define RESERVED_BYTES 16

/**
 * @brief The structure of the directory entry as stored in the filesystem is as follows:
- char name[32]: null-terminated file name 
  name[0] also serves as a special marker:
    – 0: end of directory
    – 1: deleted entry; the file is also deleted
    – 2: deleted entry; the file is still being used
- uint32_t size: number of bytes in file
- uint16_t firstBlock: the first block number of the file (undefined if size is zero)
  The block index of data region starts from 1. If the firstBlock is 0, it means that this is an empty file which has not occupied any data region block.
- uint8_t type: the type of the file, which will be one of the following:
  – 0: unknown
  – 1: a regular file
  – 2: a directory file
  – 4: a symbolic link
- uint8_t perm: file permissions, which will be one of the following:
  – 0: none
  – 2: write only
  – 4: read only
  – 5: read and executable (shell scripts) 
  – 6: read and write
  – 7: read, write, and executable
- time_t mtime: creation/modification time as returned by time(2) in Linux
*/
typedef struct DirectoryEntry {
    char name[MAX_FILE_NAME_LENGTH]; // 32-byte    
    uint32_t size; // 4-byte
    uint16_t firstBlock; // 2-byte
    uint8_t type; // 1-byte
    uint8_t perm; // 1-byte
    time_t mtime; // 8-byte
    char reserved[RESERVED_BYTES];   // 16-byte reserved space for extension
} DirectoryEntry;
/**
 * @brief Create a FATConfig object
 * 
 * @param name 
 * @param LSB 
 * @param MSB 
 * @return FATConfig* 
 */
FATConfig *createFATConfig(const char *name, uint16_t LSB, uint16_t MSB);
/**
 * @brief Create a FAT16 object in memory
 * 
 * @param config 
 * @return uint16_t* 
 */
uint16_t *createFAT16InMemory(FATConfig *config);
/**
 * @brief Create a FAT16 object on disk
 * 
 * @param config 
 * @return int 
 */
int createFATOnDisk(FATConfig *config);

/* ##### TODO: Thread Safety ##### */

/**
 * @brief Return the index of next empty FAT entry.
 * 
 * @param config 
 * @param FAT16 
 * @return int 
 */
int findEmptyFAT16Entry(FATConfig *config, uint16_t *FAT16);
/**
 * @brief Create a Directory Entry object
 * 
 * @param name 
 * @param size 
 * @param firstBlock 
 * @param type 
 * @param perm 
 * @return DirectoryEntry* 
 */
DirectoryEntry *createDirectoryEntry(const char *name, uint32_t size, uint16_t firstBlock, uint8_t type, uint8_t perm);
/**
 * @brief Create a File Directory On Disk object
 * 
 * @param config 
 * @param FAT16 
 * @param fileName 
 * @param fileType 
 * @param filePerm 
 * @return int 
 */
int createFileDirectoryOnDisk(FATConfig *config, uint16_t *FAT16, const char *fileName, uint8_t fileType, uint8_t filePerm);
/**
 * @brief Return the offset of the file directory entry in data region.
 * 
 * @param config 
 * @param FAT16 
 * @param fileName 
 * @return int 
 */
int findFileDirectory(FATConfig *config, uint16_t *FAT16, const char *fileName);
/**
 * @brief Read the directory entry from FAT and set it to dir.
 * 
 * @param config 
 * @param offset 
 * @param dir 
 * @return int 
 */
int readDirectoryEntry(FATConfig *config, int offset, DirectoryEntry *dir);
/**
 * @brief Write the directory entry to the offset.
 * 
 * @param config 
 * @param offset 
 * @param dir 
 * @return int 
 */
int writeFileDirectory(FATConfig *config, int offset, DirectoryEntry *dir);
/**
 * @brief Delete the file directory entry.
 * 
 * @param config 
 * @param FAT16 
 * @param directoryEntryOffset 
 * @return int 
 */
int deleteFileDirectory(FATConfig *config, uint16_t *FAT16, int directoryEntryOffset);
/**
 * @brief Delete the file directory entry by file name.
 * 
 * @param config 
 * @param FAT16 
 * @param fileName 
 * @return int 
 */
int deleteFileDirectoryByName(FATConfig *config, uint16_t *FAT16, const char *fileName);
/**
 * @brief Judge whether the directory entry is to be deleted.
 * 
 * @param config 
 * @param offset 
 * @return true 
 * @return false 
 */
bool isDirectoryEntryToDelete(FATConfig *config, int offset);
/**
 * @brief Read the file data from FAT and set it to buffer.
 * 
 * @param config 
 * @param FAT16 
 * @param startBlock 
 * @param startBlockOffset 
 * @param size 
 * @param buffer 
 * @return int 
 */
int readFAT(FATConfig *config, uint16_t *FAT16, int startBlock, int startBlockOffset, int size, char *buffer);
/**
 * @brief Write the buffer to FAT.
 * 
 * @param config 
 * @param FAT16 
 * @param startBlock 
 * @param startBlockOffset 
 * @param size 
 * @param buffer 
 * @return int 
 */
int writeFAT(FATConfig *config, uint16_t *FAT16, int startBlock, int startBlockOffset, int size, const char *buffer);
/**
 * @brief Return the offset of the file end.
 * 
 * @param config 
 * @param FAT16 
 * @param fileName 
 * @return int 
 */
int traceFileEnd(FATConfig *config, uint16_t *FAT16, const char *fileName);
/**
 * @brief Return the byte number from the file beginning to the file offset.
 * 
 * @param config 
 * @param FAT16 
 * @param directoryEntryOffset 
 * @param fileOffset 
 * @return int 
 */
int traceBytesFromBeginning(FATConfig *config, uint16_t *FAT16, int directoryEntryOffset, int fileOffset);
/**
 * @brief Return the byte number from the file offset to the file end.
 * 
 * @param config 
 * @param FAT16 
 * @param directoryEntryOffset 
 * @param fileOffset 
 * @return int 
 */
int traceBytesToEnd(FATConfig *config, uint16_t *FAT16, int directoryEntryOffset, int fileOffset);
/**
 * @brief Return the offset of the n bytes after the given file offset.
 * 
 * @param config 
 * @param FAT16 
 * @param directoryEntryOffset 
 * @param fileOffset 
 * @param n 
 * @return int 
 */
int traceOffset(FATConfig *config, uint16_t *FAT16, int directoryEntryOffset, int fileOffset, int n);

#endif