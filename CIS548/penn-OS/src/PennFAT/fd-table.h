/**
 * @file fd-table.h
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef FD_TABLE_H
#define FD_TABLE_H

#include "utils.h"
#include "FAT.h"

extern FATConfig *fs_FATConfig;
extern uint16_t *fs_FAT16InMemory;

#define MAX_FILE_DESCRIPTOR 8

/* file descriptor 0,1,2 were reserved */
#define F_STDIN_FD 0
#define F_STDOUT_FD 1
#define F_ERROR 2
#define F_MIN_FD 3

#define F_WRITE 0
#define F_READ 1
#define F_APPEND 2

/* We use linked list to implement the file descriptor table. */


/**
 * @brief There are three open mode supported by PennFAT: F_WRITE, F_READ and F_APPEND.
According to ed #953, each file can only be read/write exclusivly which means only one instance can open() a file at a time.
Under F_APPEND mode, the fileOffset will be set to the end of the file initially and it can only be increased.
Under F_WRITE/F_APPEND mode, if the fileOffset is set to the position beyond the file size, the file system will occupy the space for the gap.
As a result, the size of the file will increase, however, the gap space may contain uninitialized contents.
Under F_READ mode, if the fileOffset is set to the position beyond the file size, f_read() will read nothing.
*/
typedef struct FdNode {
    int openMode;
    int directoryEntryOffset; // directory entry location
    /* If a file is an empty file, the fileOffset will be set to 0. */
    int fileOffset;

    struct FdNode* prev;
    struct FdNode* next;
} FdNode;

/**
 * @brief The file descriptor table is a linked list of FdNode.
 * 
 */
typedef struct FdTable {
    FdNode *head;
    FdNode *tail;
} FdTable;

/**
 * @brief Create a Fd Node object
 * 
 * @param openMode 
 * @param directoryEntryOffset 
 * @param fileOffset 
 * @return FdNode* 
 */
FdNode *createFdNode(int openMode, int directoryEntryOffset, int fileOffset);
/**
 * @brief Initialize the file descriptor table.
 * 
 * @param fdTable 
 * @return int 
 */
int initFdTable(FdTable *fdTable);
/**
 * @brief Clear the file descriptor table.
 * 
 * @param fdTable 
 * @return int 
 */
int clearFdTable(FdTable *fdTable);
/**
 * @brief Append a new FdNode to the file descriptor table.
 * 
 * @param fdTable 
 * @param newNode 
 * @return int 
 */
int appendFdTable(FdTable *fdTable, FdNode *newNode);
/**
 * @brief Remove a FdNode from the file descriptor table.
 * 
 * @param fdNode 
 * @return int 
 */
int removeFdNode(FdNode *fdNode);
/**
 * @brief Check if the file is being used by any file descriptor.
 * 
 * @param fdTable 
 * @param directoryEntryOffset 
 * @return true 
 * @return false 
 */
bool isFileBeingUsed(FdTable *fdTable, int directoryEntryOffset);
/**
 * @brief Check if the file is being written by any file descriptor.
 * 
 * @param fdTable 
 * @param directoryEntryOffset 
 * @return true 
 * @return false 
 */
bool isFileBeingWritten(FdTable *fdTable, int directoryEntryOffset);
/**
 * @brief Find the first available file descriptor.
 * 
 * @param fds 
 * @return int 
 */
int findAvailableFd(FdNode **fds);

#endif