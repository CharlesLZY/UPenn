#ifndef SHELL_BEHAVIOR_H
#define SHELL_BEHAVIOR_H

#include "utils.h"

#include "job-control.h"

#define MAX_LINE_LENGTH 1024

typedef enum {
    EXIT_SHELL,
    EMPTY_LINE,
    EXECUTE_COMMAND
} LineType;

extern RunningMode runningMode;
extern JobList _jobList; // store all background job
extern JobList _finishedJobList; // store all finished background job for async mode
extern pid_t fgPid; // running foreground process

void setRunningMode(int argc, char **argv);

void writePrompt();

void setSignalHandler();
void SIGINTHandler(int sig);
void SIGTSTPHandler(int sig);
void SIGTTINHandler(int sig);
void SIGCONTHandler(int sig);
void SIGCHLDHandler(int sig);

void readUserInput(char **line); 
LineType parseUserInput(char *line);
LineType readAndParseUserInput(char **line);
LineType readAndParseFileInput(char **line, size_t *len);

int parseLine(char *line, struct parsed_command **cmd);
void executeLine(struct parsed_command *cmd);
void handlePipeline(struct parsed_command *cmd);

#endif