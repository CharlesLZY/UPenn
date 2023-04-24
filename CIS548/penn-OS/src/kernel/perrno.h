#ifndef PERRNO_H
#define PERRNO_H

#include "stdio.h"
#include "errno.h"

/**
 * @file perrno.h
 * @brief Error codes and messages in PennOS
 */

/**
 * @brief error codes in PennOS
 */

#define P_SUCCESS 0
#define P_NO_SUCH_PROCESS -1
#define P_NO_SUCH_FILE -2
#define P_FAIL_TO_INITIALIZE_QUEUE -3
#define P_SHOULD_NOT_KILL_SHELL -4
#define P_NODE_IS_NULL -5
#define P_PARENT_IS_NULL -6
#define P_PROCESS_IS_NULL -7
#define P_PROCESS_NOT_IN_READY_QUEUE -8
#define P_PROCESS_NOT_IN_STOPPED_QUEUE -9
#define P_FAIL_TO_CLEANUP -10

/**
 * @brief set p_error to specified error number
 * @param err_num error number
 */
void p_set_errno(int errnum);

/**
 * @brief log error message and set error number
 * @param msg error message that user wants to display
 */
void p_perror(const char* error_msg);

/**
 * @brief reset the error number to 0
 */
void p_reset_errno();

#endif