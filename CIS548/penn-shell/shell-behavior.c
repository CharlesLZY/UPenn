#include "shell-behavior.h"


void setRunningMode(int argc, char **argv) {
    char asyncFlag[] = "--async";

    if (ftell(stdin) == 0) {
       runningMode = NON_INTERACTIVE;
    } else {
        if (argc == 2 && strcmp(argv[1], asyncFlag) == 0) {
           runningMode = INTERACTIVE_ASYNC;
        } else {
           runningMode = INTERACTIVE_SYNC;
        }
    }
}

void writePrompt() {
    if (write(STDERR_FILENO, PROMPT, strlen(PROMPT)) == -1) {
        perror("Failed to write the prompt.");
        exit(EXIT_FAILURE);
    }
}


void setSignalHandler() {
    if (signal(SIGINT, SIGINTHandler) == SIG_ERR) {
        perror("Failed to set SIGINT handler.");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGTSTP, SIGTSTPHandler) == SIG_ERR) {
        perror("Failed to set SIGTSTP handler.");
        exit(EXIT_FAILURE);
    }

    /* cater to valgrind */
    if (signal(SIGTTIN, SIGTTINHandler) == SIG_ERR) {
        perror("Failed to set SIGTTIN handler.");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGCONT, SIGCONTHandler) == SIG_ERR) {
        perror("Failed to set SIGCONT handler.");
        exit(EXIT_FAILURE);
    }

    if (runningMode == INTERACTIVE_ASYNC) {
        if (signal(SIGCHLD, SIGCHLDHandler) == SIG_ERR) {
            perror("Failed to set SIGCHLD handler.");
            exit(EXIT_FAILURE);
        }
    }
}

/* When Ctrl+C is pressed, it will send SIGINT to the running foreground job process */
void SIGINTHandler(int sig) {
    if (runningMode != NON_INTERACTIVE) {
        if (fgPid > 0) { // shell process
            
        } else if (fgPid == 0) { // foreground job process
            killpg(fgPid, SIGKILL);
        } else { // shell process with no foreground job process
            writeNewline();
            writePrompt();
        }
    } 
}

/* When Ctrl+Z is pressed, it will send SIGTSTP to the running foreground job process */
void SIGTSTPHandler(int sig) {
    if (runningMode != NON_INTERACTIVE) {
        if (fgPid > 0) { // shell process

        } else if (fgPid == 0) { // foreground job process
            #ifdef DEBUG_INFO
            pid_t pid = getpid();
            printf("Pid: %d received SIGTSTP.\n", pid);
            #endif
            killpg(fgPid, SIGSTOP);
        } else { // shell process with no foreground job process
            writeNewline();
            writePrompt();
        }
    } 
}

void SIGTTINHandler(int sig) {
    pid_t pid = getpid();

    #ifdef DEBUG_INFO
    printf("Pid: %d received SIGTTIN.\n", pid);
    printf("Pid: %d is stopped because of TC.\n", pid);
    #endif

    killpg(pid, SIGSTOP);
}

void SIGCONTHandler(int sig) {
    #ifdef DEBUG_INFO
    pid_t pid = getpid();
    printf("Pid: %d received SIGCONT.\n", pid);
    #endif
}

void SIGCHLDHandler(int sig) {
    if (runningMode == INTERACTIVE_ASYNC) {
        int wstatus;
        pid_t pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED);

        if (pid > 0) {
            if (WIFEXITED(wstatus)) {
                Job *finishedJob = popJobList(&_jobList, pid);
                finishedJob->state = FINISHED;
                appendJobList(&_finishedJobList, finishedJob);
            }
        }

        #ifdef DEBUG_INFO
        if (WIFEXITED(wstatus)) {
            printf("Current pid: %d waitpid: %d EXIT\n", getpid(), pid);
        } else if (WIFSIGNALED(wstatus)) {
            printf("Current pid: %d waitpid: %d TERMINATED\n", getpid(), pid);
        } else if (WIFSTOPPED(wstatus)) {
            printf("Current pid: %d waitpid: %d STOPPED\n", getpid(), pid);  
        } else {
            printf("Current pid: %d waitpid: %d WTF\n", getpid(), pid);
        }
        #endif
    }
}

void readUserInput(char **line) {
    char inputBuffer[MAX_LINE_LENGTH];
    int numBytes = read(STDIN_FILENO, inputBuffer, MAX_LINE_LENGTH);
    if (numBytes == -1) {
        perror("Failed to read the user input.");
        exit(EXIT_FAILURE);
    }
   
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
    LineType lineType = EXECUTE_COMMAND;

    if (line == NULL) {
        lineType = EXIT_SHELL;
        return lineType;
    }

    if (line[0] == '\0') {
        lineType = EMPTY_LINE;
    } 

    return lineType;
}

LineType readAndParseUserInput(char **line) {
    readUserInput(line);
    return parseUserInput(*line);
}

LineType readAndParseFileInput(char **line, size_t *len) {
    LineType lineType;

    ssize_t numBytes = getline(line, len, stdin);

    if (numBytes <= 0) {
        lineType = EXIT_SHELL;
    } else {
        lineType = EXECUTE_COMMAND;
    }
    return lineType;
}

/*
In project 1, we assume that input redirection needs to go into first command 
and output redirection needs to come out of last command. (According to ed#355)
As long as the command line can not be recognized by the parser, it will not be executed.
*/
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

void executeLine(struct parsed_command *cmd) {
    pid_t pid = fork(); // job process
    fgPid = pid;

    if (pid == 0) { // child process
        handlePipeline(cmd);
        if (runningMode != NON_INTERACTIVE) {
            exit(EXIT_SUCCESS);
        } else {
            /* Working around solution for the bug in the non-interactive mode */
            abort(); // abort() will terminate the process by raising a SIGABRT signal
        }
        /* The child process should end here to avoid fork spawning. */
    }

    if (pid > 0) { // parent process
        setpgid(pid, pid);

        if (cmd->is_background == false || runningMode == NON_INTERACTIVE) { // In non-interactive mode, & will be ignored
            
            if (runningMode != NON_INTERACTIVE) {
                tcsetpgrp(STDIN_FILENO, pid); // delegate the terminal control
            }

            int wstatus = 0;
            do {
                waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);
            } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus) && !WIFSTOPPED(wstatus));

            if (runningMode != NON_INTERACTIVE) {
                signal(SIGTTOU, SIG_IGN); // ignore the signal from UNIX when the main process come back from the background to get the terminal control
                tcsetpgrp(STDIN_FILENO, getpid()); // give back the terminal control to the main process
            }

            if (WIFEXITED(wstatus)) {
                free(cmd);
            }
            else if (WIFSIGNALED(wstatus)) {
                if (runningMode != NON_INTERACTIVE) {
                    writeNewline();
                }
                free(cmd);

            } else if (WIFSTOPPED(wstatus)) {
                Job *newBackgroundJob = createJob(cmd, pid, STOPPED);
                appendJobList(&_jobList, newBackgroundJob);
                writeNewline();
                writeJobState(newBackgroundJob);

                if (runningMode == INTERACTIVE_ASYNC) {
                    kill(pid, SIGCONT);
                }
            }

        } else { // cmd->is_background == true
            Job *newBackgroundJob = createJob(cmd, pid, RUNNING);
            appendJobList(&_jobList, newBackgroundJob);
            writeJobState(newBackgroundJob);
        }
        fgPid = -1; // reset the foreground process
    }
}


void handlePipeline(struct parsed_command *cmd) {
    if (cmd->num_commands > 0) { 
        int pids[cmd->num_commands];

        /* Set stdin/stdout redirection */
        if (cmd->stdin_file != NULL) {
            int inputFd = open(cmd->stdin_file, O_RDONLY);
            dup2(inputFd, STDIN_FILENO);
        }

        if (cmd->stdout_file != NULL) {
            int createMode = O_RDWR | O_CREAT;
            if (cmd->is_file_append) {
                createMode |= O_APPEND;
            } else {
                createMode |= O_TRUNC;
            }
            int outputFd = open(cmd->stdout_file, createMode, 0644);
            dup2(outputFd, STDOUT_FILENO);
        }

        /* create n-1 pipes for n commands in the pipeline */
        int pfds[cmd->num_commands - 1][2]; // pipe file descriptors
        for (int i = 0; i < cmd->num_commands - 1; i++) {
            pipe(pfds[i]);
        }

        /*
        cmd1 ------- write -----
                    pfds[0]    |
                --> [   |   ]<--
                |
                ----- read ----- cmd2 ----- write ------
                                                pfds[1]   |
                                        --> [   |   ]<--
                                        |
                                        ----- read ----- cmd3
                                                                ...
        */

        for (int idx = 0; idx < cmd->num_commands; idx++) {
            pid_t pid = fork();
            if (pid == 0) { // child process
                if (idx != 0) {
                    dup2(pfds[idx -1][0], STDIN_FILENO);
                }
                if (idx != cmd->num_commands - 1) {
                    dup2(pfds[idx][1], STDOUT_FILENO);
                }

                for (int i = 0; i < cmd->num_commands - 1; i++) {
                    close(pfds[i][0]);
                    close(pfds[i][1]);
                }

                execvp(cmd->commands[idx][0], cmd->commands[idx]);
                /* if the command is executed successfully , the child process will end here.*/
                perror(cmd->commands[idx][0]);
                exit(EXIT_FAILURE);

            }
            pids[idx] = pid;
        }
        
        // close all pipe ports
        for (int i = 0; i < cmd->num_commands - 1; i++) {
            close(pfds[i][0]);
            close(pfds[i][1]);
        }

        int wstatus;
        for (int i = 0; i < cmd->num_commands; i++) {
            do {
                waitpid(pids[i], &wstatus, WUNTRACED | WCONTINUED);
            } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
        }
    }
}