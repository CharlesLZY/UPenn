/**
 * @file pennFAT.h
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef PENNFAT_H
#define PENNFAT_H

#include "filesys.h"
#include "signal.h"

#define PF_MAX_BUFFER_SIZE 32512
#define PF_MAX_FILE_NUM 4

extern FATConfig *fs_FATConfig;
extern uint16_t *fs_FAT16InMemory;

/**
 * @brief PennFAT write mode.
 * 
 */
typedef enum {
    PF_OVERWRITE,
    PF_APPEND,
    PF_STDOUT
} PF_WRITEMODE;

/**
 * @brief Check if the PennFAT is mounted.
 * 
 * @return true 
 * @return false 
 */
bool pf_isMounted();
/**
 * @brief Read a file.
 * 
 * @param fileName 
 * @param size 
 * @param buffer 
 * @return int 
 */
int pf_readFile(const char *fileName, int size, char *buffer);
/**
 * @brief Write a file.
 * 
 * @param fileName 
 * @param size 
 * @param buffer 
 * @param mode 
 * @return int 
 */
int pf_writeFile(const char *fileName, int size, const char *buffer, PF_WRITEMODE mode);
;/**
 * @brief Make a PennFAT file system.
 * 
 * @param fsName 
 * @param BLOCKS_IN_FAT 
 * @param BLOCK_SIZE_CONFIG 
 * @return int 
 */
int pf_mkfs(const char *fsName, int BLOCKS_IN_FAT, int BLOCK_SIZE_CONFIG);
/**
 * @brief Mount a PennFAT file system.
 * 
 * @param fsName 
 * @return int 
 */
int pf_mount(const char *fsName);
/**
 * @brief Unmount a PennFAT file system.
 * 
 * @return int 
 */
int pf_umount();
/**
 * @brief Create a file.
 * 
 * @param fileName 
 * @return int 
 */
int pf_touch(const char *fileName);
/**
 * @brief Remove a file.
 * 
 * @param fileName 
 * @return int 
 */
int pf_rm(const char *fileName);
/**
 * @brief Rename a file.
 * 
 * @param src 
 * @param dst 
 * @return int 
 */
int pf_mv(const char *src, const char *dst);
/**
 * @brief List all files.
 * 
 * @return int 
 */
int pf_ls();
/**
 * @brief Change the permission of a file.
 * 
 * @param fileName 
 * @param perm 
 * @return int 
 */
int pf_chmod(const char *fileName, uint8_t perm);
/**
 * @brief Cat files.
 * 
 * @param fileNames 
 * @param fileNum 
 * @param size 
 * @param buffer 
 * @return int 
 */
int pf_catFiles(char **fileNames, int fileNum, int *size, char *buffer);


#define MAX_LINE_LENGTH 4096
#define MAX_ARGS_NUM 8

#define EXIT_SHREDDER -1
#define EMPTY_LINE 0
#define EXECUTE_COMMAND 1
/**
 * @brief Signal handler for SIGINT.
 * 
 * @param sig 
 */
void SIGINTHandler(int sig);
/**
 * @brief Read input utility function.
 * 
 * @param inputBuffer 
 * @return char* 
 */
char *readInput(char *inputBuffer);
/**
 * @brief Parse input utility function.
 * 
 * @param userInput 
 * @param argsBuffer 
 * @param argNum 
 * @return int 
 */
int parseInput(char *userInput, char **argsBuffer, int *argNum);

#endif