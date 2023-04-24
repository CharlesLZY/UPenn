/**
 * @file behavior.c
 * @author Shuo Sun (sunshuo@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "behavior.h"

void writePrompt() {
    f_write(F_ERROR, PROMPT, strlen(PROMPT));
}

void readUserInput(char **line) {
    char inputBuffer[S_MAX_BUFFER_SIZE];
    int numBytes = f_read(F_STDIN_FD, S_MAX_BUFFER_SIZE, inputBuffer);
   
    if (numBytes == 0) { // read nothing but EOF (Ctrl + D at the beginning of the input line)
        *line = NULL;
    } else {
        char *userInput = NULL;
        if (inputBuffer[numBytes - 1] != '\n') { // input ended with EOF (Ctrl + D)
            userInput = malloc(1);
            userInput[0] = '\0';
            *line = userInput;
        } else {
            if (numBytes == 1) { // read nothing but '/n'
                userInput = malloc(2);
                userInput[0] = '\n';
                userInput[1] = '\0';
                *line = userInput;
            }
            else {
                userInput = malloc(numBytes);
                for (int i = 0; i < numBytes; i++) {
                    if (inputBuffer[i] == '\n') {
                        userInput[i] = '\0';
                    } 
                    else {
                        userInput[i] = inputBuffer[i];
                    }
                }
                *line = userInput;
            }
        }
    }
}

LineType parseUserInput(char *line) {
    LineType lineType = S_EXECUTE_COMMAND;

    if (line == NULL) {
        lineType = S_EXIT_SHELL;
        return lineType;
    }

    if ((line[0] == '\0') || (line[0] == '\n') ) {
        lineType = S_EMPTY_LINE;
    } 

    return lineType;
}

LineType readAndParseUserInput(char **line) {
    readUserInput(line);
    return parseUserInput(*line);
}

int parseLine(char *line, struct parsed_command **cmd) {
    int res = parse_command(line, cmd);
    if (res < 0) {
        perror("parse_command");
    }
    else if (res > 0) {
        printf("syntax error: %d\n", res);
    }
    return res;
}

ProgramType parseProgramType(struct parsed_command *cmd) {
    if (strcmp(*cmd->commands[0], "cat") == 0) {
        return CAT;
    } else if (strcmp(*cmd->commands[0], "sleep") == 0) {
        return SLEEP;
    } else if (strcmp(*cmd->commands[0], "busy") == 0) {
        return BUSY;
    } else if (strcmp(*cmd->commands[0], "echo") == 0) {
        return ECHO;
    } else if (strcmp(*cmd->commands[0], "ls") == 0) {
        return LS;
    } else if (strcmp(*cmd->commands[0], "touch") == 0) {
        return TOUCH;
    } else if (strcmp(*cmd->commands[0], "mv") == 0) {
        return MV;
    } else if (strcmp(*cmd->commands[0], "cp") == 0) {
        return CP;
    } else if (strcmp(*cmd->commands[0], "rm") == 0) {
        return RM;
    } else if (strcmp(*cmd->commands[0], "chmod") == 0) {
        return CHMOD;
    } else if (strcmp(*cmd->commands[0], "ps") == 0) {
        return PS;
    } else if (strcmp(*cmd->commands[0], "zombify") == 0) {
        return ZOMBIFY;
    } else if (strcmp(*cmd->commands[0], "orphanify") == 0) {
        return ORPHANIFY;
    } else if (strcmp(*cmd->commands[0], "hang") == 0) {
        return HANG;
    } else if (strcmp(*cmd->commands[0], "nohang") == 0) {
        return NOHANG;
    } else if (strcmp(*cmd->commands[0], "recur") == 0) {
        return RECUR;
    } else if (strcmp(*cmd->commands[0], "test") == 0) {
        return TEST;
    } else {
        return UNKNOWN;
    }
}

int executeLine(struct parsed_command *cmd, int priority) {

    int fd_in = F_STDIN_FD;
    int fd_out = F_STDOUT_FD;
    
    if (cmd->stdin_file != NULL) {
        int res = f_open(cmd->stdin_file, F_READ);
        if (res < 0) {
            printf("Fail to open file: %s\n", cmd->stdin_file);
            return -1;
        }
        fd_in = res;
    }

    if (cmd->stdout_file != NULL) {
        if (cmd->is_file_append) {
            fd_out = f_open(cmd->stdout_file, F_APPEND);
        } else {
            fd_out = f_open(cmd->stdout_file, F_WRITE);
        }
    }
    ProgramType programType = parseProgramType(cmd);;
    pid_t pid = executeProgram(programType, *cmd->commands, fd_in, fd_out);
    
    if (!cmd->is_background) {
        fgPid = pid;
    }

    // TODO: bug in p_nice example nice 1 sleep 1
    if (priority != MID) {
        p_nice(pid, priority);
    }
    
    if (!cmd->is_background) {
        int wstatus;
        p_waitpid(pid, &wstatus, false);

        fgPid = 1;
        if (cmd->stdin_file != NULL) {
            f_close(fd_in);
        }
        if (cmd->stdout_file != NULL) {
            f_close(fd_out);
        }

        if (W_WIFEXITED(wstatus)) {
            free(cmd);
        } else if (W_WIFSIGNALED(wstatus)) {
            free(cmd);
        } else if (W_WIFSTOPPED(wstatus)) {
            Job *newBackgroundJob = createJob(cmd, pid, JOB_STOPPED);
            appendJobList(&_jobList, newBackgroundJob);
            writeNewline();
            writeJobState(newBackgroundJob);
        }
        
    } else {
        if (programType == CAT) {
            // CAT will require Terminal Control of stdin, so it should be stopped
            p_kill(pid, S_SIGSTOP);
        } 
        Job *newBackgroundJob = createJob(cmd, pid, JOB_RUNNING);
        appendJobList(&_jobList, newBackgroundJob);
        writeJobState(newBackgroundJob);
    }

    return 0; 
}

pid_t executeProgram(ProgramType programType, char **argv, int fd_in, int fd_out) {
    pid_t pid = -1;
    /* Switch case on user programs */
    switch (programType) {
        case CAT:
            pid = p_spawn(s_cat, argv, fd_in, fd_out);
            break;
        case SLEEP:
            pid = p_spawn(s_sleep, argv, fd_in, fd_out);
            break;
        case BUSY:
            pid = p_spawn(s_busy, argv, fd_in, fd_out);
            break;
        case ECHO:
            pid = p_spawn(s_echo, argv, fd_in, fd_out);
            break;
        case LS:
            pid = p_spawn(s_ls, argv, fd_in, fd_out);
            break;
        case TOUCH:
            pid = p_spawn(s_touch, argv, fd_in, fd_out);
            break;
        case MV:
            pid = p_spawn(s_mv, argv, fd_in, fd_out);
            break;
        case CP:
            pid = p_spawn(s_cp, argv, fd_in, fd_out);
            break;
        case RM:
            pid = p_spawn(s_rm, argv, fd_in, fd_out);
            break;
        case CHMOD:
            pid = p_spawn(s_chmod, argv, fd_in, fd_out);
            break;
        case PS:
            pid = p_spawn(s_ps, argv, fd_in, fd_out);
            break;
        case ZOMBIFY:
            pid = p_spawn(s_zombify, argv, fd_in, fd_out);
            break;
        case ORPHANIFY:
            pid = p_spawn(s_orphanify, argv, fd_in, fd_out);
            break;
        case HANG:
            pid = p_spawn(s_hang, argv, fd_in, fd_out);
            break;
        case NOHANG:
            pid = p_spawn(s_nohang, argv, fd_in, fd_out);
            break;
        case RECUR:
            pid = p_spawn(s_recur, argv, fd_in, fd_out);
            break;
        case TEST:
            pid = p_spawn(s_test, argv, fd_in, fd_out);
            break;
        default:
            pid = p_spawn(executeScript, argv, fd_in, fd_out);
    }
    return pid;
}

void executeScript(char *argv[]) {
    char *fileName = argv[0];
    int fd = f_open(fileName, F_READ);
    char scriptBuffer[S_MAX_BUFFER_SIZE];
    memset(scriptBuffer, 0, S_MAX_BUFFER_SIZE);
    f_read(fd, S_MAX_BUFFER_SIZE, scriptBuffer);
    f_close(fd);

    struct parsed_command *cmd;
    char* token;
    token = strtok(scriptBuffer, "\n");
    while (token != NULL) {
        int res = parseLine(token, &cmd);
        if (res == 0) {
            // TODO: handle priority
            executeLine(cmd, MID);
        }
        token = strtok(NULL, "\n");
    }
}