/**
 * @file shell.c
 * @author Shuo Sun (sunshuo@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "shell.h"
#include "scheduler.h"

JobList _jobList; // store all background job
pid_t fgPid = 1;

void shell_process() {

    char *line = NULL;
    LineType lineType;
    while (true) {
        p_reset_errno();
        writePrompt();
        lineType = readAndParseUserInput(&line);
        pollBackgroundProcesses();
        
        if (lineType == S_EXIT_SHELL) {
            return;
        }
        else if (lineType == S_EMPTY_LINE) {
            writeNewline();
        }
        else {
            struct parsed_command *cmd;
            int res = parseLine(line, &cmd);
            if (res == 0) {
                if (isBuildinCommand(cmd)) {
                    CommandType cmdType = executeBuiltinCommand(cmd);
                    if (cmdType == NICE) {
                        int priority = atoi(cmd->commands[0][1]);
                        cmd->commands[0] += 2;
                        executeLine(cmd, priority);
                    }
                    if (cmdType == LOGOUT) {
                        p_exit();
                    }
                } else {
                    if (isKnownProgram(cmd)) {
                        executeLine(cmd, MID);
                    } else {
                        if (cmd->num_commands != 1) {
                            printf("Error: Unexcepted Input\n");
                            continue;
                        } 
                        if (f_isExecutable(cmd->commands[0][0]) == false){
                            printf("Error: Cannot execute %s\n", cmd->commands[0][0]);
                            continue;
                        }
                        executeLine(cmd, MID);
                    }
                }
            }
        }
        free(line);
    }

    clearJobList(&_jobList);
}


bool isBuildinCommand(struct parsed_command *cmd) {
    if (strcmp(*cmd->commands[0], "bg") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "fg") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "jobs") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "nice") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "nice_pid") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "man") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "kill") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "logout") == 0) {
        return true;
    } else {
        return false;
    }
}

bool isKnownProgram(struct parsed_command *cmd) {
    if (strcmp(*cmd->commands[0], "cat") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "sleep") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "busy") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "echo") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "ls") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "touch") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "mv") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "cp") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "rm") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "chmod") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "ps") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "zombify") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "orphanify") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "hang") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "nohang") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "recur") == 0) {
        return true;
    } else if (strcmp(*cmd->commands[0], "test") == 0) {
        return true;
    } else {
        return false;
    }
}

int shell_init(int argc, const char **argv) {
    // initialize kernel
    if (kernel_init() == FAILURE ) {
        return FAILURE;
    }
    // initialize scheduler
    if (scheduler_init() == FAILURE ) {
        return FAILURE;
    }

    // initialize signal handlers
    if (register_signals() == FAILURE ) {
        return FAILURE;
    }

    // initialize logger
    if (argc == 2) {
        if (log_init(NULL) == FAILURE) {
            return FAILURE;
        };
    } else {
        if (log_init(argv[2]) == FAILURE) {
            return FAILURE;
        };
    }
    
    
    // initialize file system
    if (fs_mount(argv[1]) == -1) {
        return FAILURE;
    } 
    // fs_mount("test");

    // initialize job list
    initJobList(&_jobList);

    // initialize main context
    getcontext(&main_context);
    sigemptyset(&(main_context.uc_sigmask));
    set_stack(&(main_context.uc_stack));
    main_context.uc_link = NULL;

    active_process = NULL;
    p_active_context = NULL;

    // init shell ucontext
    ucontext_t shell_context;
    getcontext(&shell_context);
    sigemptyset(&(shell_context.uc_sigmask));
    set_stack(&(shell_context.uc_stack));
    shell_context.uc_link = &main_context;

    if (makeContext(&shell_context, shell_process, 0, NULL, NULL) == FAILURE) {
        return FAILURE;
    }

    pcb *shell_pcb = new_pcb(&shell_context, lastPID++);
    shell_pcb->priority = -1;   // the default is 0, but we want -1
    printf("shell pid: %i\n", shell_pcb->pid);
    shell_pcb->pname = malloc(sizeof(char)*(strlen("shell"))+1);
    strcpy(shell_pcb->pname, "shell");
    pcb_node *shell_node = new_pcb_node(shell_pcb);

    enqueue_by_priority(ready_queue, HIGH, shell_node);

    printf("shell process initialized\n");

    return SUCCESS;
}
