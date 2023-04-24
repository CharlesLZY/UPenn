#ifndef SHREDDER_BEHAVIOR_H
#define SHREDDER_BEHAVIOR_H

#include <unistd.h> // system calls
#include <sys/wait.h> // wait
#include <signal.h> // signal
#include <stdlib.h> // malloc, free, atoi
#include <string.h> // strlen, strtok
#include <stdio.h> // perror

#define MAX_LINE_LENGTH 4096
#define MAX_ARGS_NUM 1024

#define EXIT_SHREDDER -1
#define EMPTY_LINE 0
#define EXECUTE_COMMAND 1

/*
Parse the arguments and return the time limit.
If there is no argument, then the time limit will be 0 which means the runtime of a single command is unlimited.
If there are extra arguments or the argument is not a positive number, it will print the usage info and exit the shredder.
*/
int parseArgument(int argc, char **argv);

/*
Print the usage info: "usage: ./penn-shredder [time limit]"
*/
void writeUsageInfo();

/*
Print the prompt: "penn-shredder\# " which is defined in the Makefile
*/
void writePrompt();

/*
Print the catchphrase: "Bwahaha ... Tonight, I dine on turtle soup!\n" which is defined in the Makefile
*/
void writeCatchphrase();

/*
Print a new line "\n".
*/
void writeNewline();

/*
Read the user input and store it in the inputBuffer.
If the input is nothing but an EOF which means the user presses Ctrl-D at the beginning of the input line, return NULL.
If the input is ended with an EOF, return an empty user input.
Otherwise, return the user input.
Note, the user input is malloc-ed and needs to be freed after execution.
*/
char *readInput(char *inputBuffer);

/*
Trim the whitespaces in the user input and split the raw inputs to arguments.
The arguments will be stored into argsBuffer.
If the userInput is NULL, return EXIT_SHREDDER. 
If the userInput is an empty string, return EMPTY_LINE.
Otherwise, return EXECUTE_COMMAND.
*/
int parseInput(char *userInput, char **argsBuffer);

/*
Fork a child process to execute the user command.
After the command is executed, the userInput will be freed.
The global variable childPid will be used to store the pid of child process.
The alarm() will be set according to the timeLimit.
*/
void executeCommand(char *userInput, char **argsBuffer, int timeLimit);

/*
Set the handlers of SIGALRM and SIGINT.
*/
void setSignalHandler();

/*
The main process will set an alarm.
When the main process receives the SIGALRM, it will write the catchphrase.
For the regular credit, the main process will kill the child process when it receives the SIGALRM.
For the additional credit, the child process will set an alarm for it self, when it receives the SIGALRM, it will exit by itself.
*/
void SIGALRMHandler(int sig);

/*
When the main process receives the SIGINT, it will write a new line.
If there is no running child process, the main process will write the prompt again.
For the regular credit, the main process will kill the child process when it receives the SIGALRM.
For the additional credit, the child process will set an alarm for it self, when it receives the SIGALRM, it will exit by itself.
*/
void SIGINTHandler(int sig);

#endif