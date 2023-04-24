/**
 * @file programs.h
 * @author Shuo Sun (sunshuo@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef PROGRAMS_H
#define PROGRAMS_H

#include "behavior.h"
#include "stress.h"
#include "../PennFAT/pennFAT.h"

#define READ_BUFFER_SIZE 4096

/* Known user programs */
/**
 * @brief Shell cat command.
 * 
 * @param argv 
 */
void s_cat(char *argv[]);
/**
 * @brief Shell sleep command.
 * 
 * @param argv 
 */
void s_sleep(char *argv[]);
/**
 * @brief Shell busy command.
 * 
 * @param argv 
 */
void s_busy(char *argv[]);
/**
 * @brief Shell echo command.
 * 
 * @param argv 
 */
void s_echo(char *argv[]);
/**
 * @brief Shell ls command.
 * 
 * @param argv 
 */
void s_ls(char *argv[]);
/**
 * @brief Shell touch command.
 * 
 * @param argv 
 */
void s_touch(char *argv[]);
/**
 * @brief Shell mv command.
 * 
 * @param argv 
 */
void s_mv(char *argv[]);
/**
 * @brief Shell cp command.
 * 
 * @param argv 
 */
void s_cp(char *argv[]);
/**
 * @brief Shell rm command.
 * 
 * @param argv 
 */
void s_rm(char *argv[]);
/**
 * @brief Shell chmod command.
 * 
 * @param argv 
 */
void s_chmod(char *argv[]);
/**
 * @brief Shell ps command.
 * 
 * @param argv 
 */
void s_ps(char *argv[]);
/**
 * @brief Shell kill command.
 * 
 * @param argv 
 */
void s_kill(char *argv[]);
/**
 * @brief Shell zombify command.
 * 
 * @param argv 
 */
void s_zombify(char *argv[]);
/**
 * @brief Shell orphanify command.
 * 
 * @param argv 
 */
void s_orphanify(char *argv[]);
/**
 * @brief Shell hang command.
 * 
 * @param argv 
 */
void s_hang(char *argv[]);
/**
 * @brief Shell nohang command.
 * 
 * @param argv 
 */
void s_nohang(char *argv[]);
/**
 * @brief Shell recur command.
 * 
 * @param argv 
 */
void s_recur(char *argv[]);
/**
 * @brief Our shell test command.
 * 
 * @param argv 
 */
void s_test(char *argv[]);

#endif