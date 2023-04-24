/**
 * @file interface.h
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef FILESYS_INTERFACE_H
#define FILESYS_INTERFACE_H

#define F_STDIN_FD 0
#define F_STDOUT_FD 1
#define F_ERROR 2

#define F_WRITE 0
#define F_READ 1
#define F_APPEND 2

#define F_SEEK_SET 0
#define F_SEEK_CUR 1
#define F_SEEK_END 2

#define F_SUCCESS 0
#define F_FAILURE -1

#include "../kernel/utils.h"

/**
 * @brief Open a file with the given mode.
 * 
 * @param fname 
 * @param mode 
 * @return int 
 */
int f_open(const char *fname, int mode);
/**
 * @brief Close a file.
 * 
 * @param fd 
 * @return int 
 */
int f_close(int fd);
/**
 * @brief Read n bytes from the file.
 * 
 * @param fd 
 * @param n 
 * @param buf 
 * @return int 
 */
int f_read(int fd, int n, char *buf);
/**
 * @brief Write n bytes to the file.
 * 
 * @param fd 
 * @param str 
 * @param n 
 * @return int 
 */
int f_write(int fd, const char *str, int n);
/**
 * @brief Seek to a position in the file.
 * 
 * @param fd 
 * @param offset 
 * @param whence 
 * @return int 
 */
int f_lseek(int fd, int offset, int whence);
/**
 * @brief Unlink a file.
 * 
 * @param fname 
 * @return int 
 */
int f_unlink(const char *fname);
/**
 * @brief List all files in the current directory.
 * 
 * @param filename 
 * @return int 
 */
int f_ls(const char *filename);
/**
 * @brief Check if a file exists.
 * 
 * @param filename 
 * @return true 
 * @return false 
 */
bool f_find(const char *filename);
/**
 * @brief Check if a file is executable.
 * 
 * @param filename 
 * @return true 
 * @return false 
 */
bool f_isExecutable(const char *filename);

#endif