/**
 * @file shell.h
 * @author Shuo Sun (sunshuo@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef SHELL_H
#define SHELL_H

#include "utils.h"
#include "job.h"
#include "user.h"
#include "behavior.h"
#include "log.h"
#include "../PennFAT/filesys.h"
#include "../PennFAT/interface.h"

/**
 * @brief The main function of shell
 * 
 */
void shell_process();
/**
 * @brief Initialize the shell
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int shell_init(int argc, const char **argv);
/**
 * @brief Check if the command is a buildin command
 * 
 * @param cmd 
 * @return true 
 * @return false 
 */
bool isBuildinCommand(struct parsed_command *cmd);
/**
 * @brief Check if the command is a known program
 * 
 * @param cmd 
 * @return true 
 * @return false 
 */
bool isKnownProgram(struct parsed_command *cmd);


#endif