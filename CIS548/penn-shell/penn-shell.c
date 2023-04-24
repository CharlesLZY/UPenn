#include "shell-behavior.h"

RunningMode runningMode;
JobList _jobList; // store all background job
JobList _finishedJobList; // store all finished background job for async mode
pid_t fgPid = -1; // -1 means no running foreground process

#ifdef DEBUG_INFO
pid_t shellPid;
#endif

int main(int argc, char **argv) {

    #ifdef DEBUG_INFO
    shellPid = getpid();
    printf("Shell Pid: %d\n", shellPid);
    #endif

    setRunningMode(argc, argv);

    setSignalHandler();

    initJobList(&_jobList);
    initJobList(&_finishedJobList);

    if (runningMode == INTERACTIVE_SYNC || runningMode == INTERACTIVE_ASYNC) {
        char *line = NULL;
        LineType lineType;

        while(1) {
            printAndClearFinishedJobList(&_finishedJobList);
            
            writePrompt();
        
            lineType = readAndParseUserInput(&line);

            pollBackgroundProcesses();

            if (lineType == EXIT_SHELL) {
                break;
            }
            else if (lineType == EMPTY_LINE) {
                writeNewline();
            }
            else {
                struct parsed_command *cmd;
                int res = parseLine(line, &cmd);
                if (res == 0) {
                    if (executeBuiltinCommand(cmd) == false) {
                        executeLine(cmd); 
                    }  
                }
            }
            free(line); // Since we use read(2) in the interactive mode, we need to free it each time
        }


    } else if (runningMode == NON_INTERACTIVE) {
        char *line = NULL;
        size_t len = 0;
        LineType lineType;

        while ((lineType = readAndParseFileInput(&line, &len)) != EXIT_SHELL) {
            struct parsed_command *cmd;
            int res = parseLine(line, &cmd);
            if (res == 0) {
                /* In non-interactive mode, builtin functions are not supported */
                executeLine(cmd);
            }
        }
        free(line);
    }

    clearJobList(&_jobList);
    clearJobList(&_finishedJobList);

    return 0;
}