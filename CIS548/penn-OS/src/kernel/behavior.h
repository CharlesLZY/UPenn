/**
 * @file behavior.h
 * @author Shuo Sun (sunshuo@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#define S_MAX_BUFFER_SIZE 4096
#define PROMPT "pennOS> "

#include "job.h"
#include "../PennFAT/interface.h"
#include "programs.h"
/**
 * @brief The type of the command line
 * 
 */
typedef enum {
    S_EXIT_SHELL,
    S_EMPTY_LINE,
    S_EXECUTE_COMMAND
} LineType;
/**
 * @brief The type of the user program
 * 
 */
typedef enum {
    CAT,
    SLEEP,
    BUSY,
    ECHO,
    LS,
    TOUCH,
    MV,
    CP,
    RM,
    CHMOD,
    PS,
    ZOMBIFY,
    ORPHANIFY,
    HANG,
    NOHANG,
    RECUR,
    TEST,
    UNKNOWN
} ProgramType;

extern JobList _jobList; // store all background job

/* Utility function for writing PROMPT */
/**
 * @brief Write the prompt to the screen
 * 
 */
void writePrompt();

/* Read and parse utilities */
/**
 * @brief Read the user input
 * 
 * @param line 
 */
void readUserInput(char **line);
/**
 * @brief Parse the user input
 * 
 * @param line 
 * @return LineType 
 */
LineType parseUserInput(char *line);
/**
 * @brief Read and parse the user input
 * 
 * @param line 
 * @return LineType 
 */
LineType readAndParseUserInput(char **line);
/**
 * @brief Parse the command line
 * 
 * @param line 
 * @param cmd 
 * @return int 
 */
int parseLine(char *line, struct parsed_command **cmd);


/* Execute a user program */
/**
 * @brief Execute the command line
 * 
 * @param cmd 
 * @return ProgramType 
 */
ProgramType parseProgramType(struct parsed_command *cmd);
/**
 * @brief Execute the command line
 * 
 * @param cmd 
 * @param priority 
 * @return int 
 */
int executeLine(struct parsed_command *cmd, int priority);
/**
 * @brief Execute the user program
 * 
 * @param programType 
 * @param argv 
 * @param fd_in 
 * @param fd_out 
 * @return pid_t 
 */
pid_t executeProgram(ProgramType programType, char **argv, int fd_in, int fd_out);
/**
 * @brief Execute the user script
 * 
 * @param argv 
 */
void executeScript(char *argv[]);



#endif