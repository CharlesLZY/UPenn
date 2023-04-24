/**
 * @file filesys.h
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef FILESYS_H
#define FILESYS_H

#include "utils.h"
#include "FAT.h"
#include "fd-table.h"

/**
 * @brief Check if the file system is mounted.
 * 
 * @return true 
 * @return false 
 */
bool isFileSystemMounted();
/**
 * @brief Check if the file name is valid.
 * 
 * @param fileName 
 * @return true 
 * @return false 
 */
bool isValidFileName(const char *fileName);

/**
 * @brief Create a file system. Return 0 if success.
 * 
 * @param fsName 
 * @param blockSizeConfig 
 * @param FATRegionBlockNum 
 * @return int 
 */
int fs_mkfs(const char *fsName, uint16_t blockSizeConfig, uint16_t FATRegionBlockNum);
/**
 * @brief Mount the file system. Return 0 if success.
 * 
 * @param fsName 
 * @return int 
 */
int fs_mount(const char *fsName);
/**
 * @brief Unmount the file system. Return 0 if success.
 * 
 * @return int 
 */
int fs_unmount();
/**
 * @brief Creates the file if it does not exist, otherwise update its timestamp. Return the offset of the file directory entry. 
 * 
 * @param fileName 
 * @return int 
 */
int fs_touch(const char *fileName);
/**
 * @brief Remove the file from FAT. Return 0 if success
 * 
 * @param fileName 
 * @return int 
 */
int fs_rm(const char *fileName);
/**
 * @brief Rename src to dst. If dst exists, remove it from FAT. Return 0 if success
 * 
 * @param src 
 * @param dst 
 * @return int 
 */
int fs_mv(const char *src, const char *dst);
/**
 * @brief Duplicate src to dst. If dst exists, replace it. Return 0 if success.
 * 
 * @param src 
 * @param dst 
 * @return int 
 */
int fs_cp(const char *src, const char *dst);
/**
 * @brief Encapsulation of readFAT(). Return 0 if success.
 * 
 * @param startBlock 
 * @param startBlockOffset 
 * @param size 
 * @param buffer 
 * @return int 
 */
int fs_readFAT(int startBlock, int startBlockOffset, int size, char *buffer);

/**
 * @brief Encapsulation of writeFAT(). Return 0 if success.
 * 
 * @param startBlock 
 * @param startBlockOffset 
 * @param size 
 * @param buffer 
 * @return int 
 */
int fs_writeFAT(int startBlock, int startBlockOffset, int size, const char *buffer);
/**
 * @brief Change the permission of the file. Return 0 if success.
 * 
 * @param fileName 
 * @param perm 
 * @return int 
 */
int fs_chmod(char *fileName, uint8_t perm);


#endif