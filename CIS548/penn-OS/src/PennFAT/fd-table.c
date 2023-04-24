/**
 * @file fd-table.c
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "fd-table.h"

FdNode *createFdNode(int openMode, int directoryEntryOffset, int fileOffset) {
    FdNode  *newNode = malloc(sizeof(FdNode));

    newNode->openMode = openMode;
    newNode->directoryEntryOffset = directoryEntryOffset;
    newNode->fileOffset = fileOffset;

    newNode->prev = NULL;
    newNode->next = NULL;
    
    return newNode;
}

int initFdTable(FdTable *fdTable) {
    fdTable->head = createFdNode(-1, -1, -1);
    fdTable->tail = createFdNode(-1, -1, -1);

    fdTable->head->prev = NULL;
    fdTable->head->next = fdTable->tail;
    fdTable->tail->prev = fdTable->head;
    fdTable->tail->next = NULL;

    return FS_SUCCESS;
}

int clearFdTable(FdTable *fdTable) {
    FdNode *nextNode = fdTable->head->next;
    FdNode *curNode = nextNode;

    while (curNode != fdTable->tail) {
        nextNode = curNode->next;
        free(curNode);
        curNode = nextNode;
    }

    free(fdTable->head);
    free(fdTable->tail);

    return FS_SUCCESS;
}

int appendFdTable(FdTable *fdTable, FdNode *newNode) {
    if (newNode == NULL) {
        printf("Error: Fail to append NULL fd node.\n");
        return FS_FAILURE;
    }

    FdNode *prevNode = fdTable->tail->prev;
    newNode->prev = prevNode;
    newNode->next = fdTable->tail;
    prevNode->next = newNode;
    fdTable->tail->prev = newNode;

    #ifdef FS_DEBUG_INFO
    printf("Fd node is appended.\n");
    #endif

    return FS_SUCCESS;
}

int removeFdNode(FdNode *fdNode) {
    if (fdNode == NULL) {
        printf("Error: Fail to remove NULL fd node.\n");
        return FS_FAILURE;
    }

    FdNode *prevNode = fdNode->prev;
    FdNode *nextNode = fdNode->next;

    if ((prevNode != NULL) && (nextNode != NULL)) {
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
    }

    free(fdNode);

    #ifdef FS_DEBUG_INFO
    printf("Fd node is removed.\n");
    #endif

    return FS_SUCCESS;
}

bool isFileBeingUsed(FdTable *fdTable, int directoryEntryOffset) {
    FdNode *curNode = fdTable->head->next;
    while (curNode != fdTable->tail) {
        if (curNode->directoryEntryOffset == directoryEntryOffset) {
            return true;
        }
        curNode = curNode->next;
    }
    return false;
}

bool isFileBeingWritten(FdTable *fdTable, int directoryEntryOffset) {
    FdNode *curNode = fdTable->head->next;
    while (curNode != fdTable->tail) {
        if ((curNode->directoryEntryOffset == directoryEntryOffset) && ((curNode->openMode == F_WRITE) || (curNode->openMode == F_APPEND))) {
            return true;
        }
        curNode = curNode->next;
    }
    return false;
}

int findAvailableFd(FdNode **fds) {
    for (int i = F_MIN_FD; i < MAX_FILE_DESCRIPTOR; i++) {
        if (fds[i] == NULL) {
            return i;
        }
    }
    return FS_NOT_FOUND;
}