#include "shredder-behavior.h"

int childPid = -1; // -1 means no child process

int parseArgument(int argc, char **argv) {
    int timeLimit = 0;
    if (argc == 2) {
        timeLimit = atoi(argv[1]); // decimal will be convert to integer
    }

    if (timeLimit < 0 || argc > 2) {
        writeUsageInfo();
        exit(0);
    }

    return timeLimit;
}

void writeUsageInfo() {
    char info[] = "Usage: ./penn-shredder [time limit]\n";
    if (write(STDERR_FILENO, info, strlen(info)) == -1) {
        perror("Failed to write the usage info.");
        exit(EXIT_FAILURE);
    }
}

void writePrompt() {
    if (write(STDERR_FILENO, PROMPT, strlen(PROMPT)) == -1) {
        perror("Failed to write the prompt.");
        exit(EXIT_FAILURE);
    }
}

void writeCatchphrase() {
    if (write(STDERR_FILENO, CATCHPHRASE, strlen(CATCHPHRASE)) == -1) {
        perror("Failed to write the catchphrase.");
        exit(EXIT_FAILURE);
    }
}

void writeNewline() {
    if (write(STDERR_FILENO, "\n", 1) == -1) {
        perror("Failed to write the newline.");
        exit(EXIT_FAILURE);
    }
}

char *readInput(char *inputBuffer) {
    int numBytes = read(STDIN_FILENO, inputBuffer, MAX_LINE_LENGTH);
    if (numBytes == -1) {
        perror("Failed to read the user input.");
        exit(EXIT_FAILURE);
    }

    if (numBytes == 0) { // read nothing but EOF (Ctrl + D at the beginning of the input line)
        return NULL;
    }

    if (inputBuffer[numBytes - 1] != '\n') { // input ended with EOF (Ctrl + D)
        char *userInput = malloc(1);
        userInput[0] = '\0';
        return userInput;
    } else {
        char *userInput = malloc(numBytes);
        for (int i = 0; i < numBytes; i++) {
            if (inputBuffer[i] == '\n') {
                userInput[i] = '\0';
            } 
            else {
                userInput[i] = inputBuffer[i];
            }
        }
        return userInput;
    }
}

int parseInput(char *userInput, char **argsBuffer) {
    if (userInput == NULL) {
        return EXIT_SHREDDER;
    }

    char *t = strtok(userInput, " \t\n"); // split the input by the whitespaces
    argsBuffer[0] = t;

    int argsNum;
    for (argsNum = 0; t != NULL; argsNum++) {
        t = strtok(NULL, " \t\n");
        if (argsNum >= MAX_ARGS_NUM) {
            break;
        }
        argsBuffer[argsNum+1] = t;
    }

    if (argsNum == 0) {
        return EMPTY_LINE;
    }

    return EXECUTE_COMMAND;
}


void executeCommand(char *userInput, char **argsBuffer, int timeLimit) {
    int wstatus;

    childPid = fork();
    if (childPid == -1) {
        perror("Failed to fork a child process.");
        exit(EXIT_FAILURE);
    }

    // child process
    if (childPid == 0) { 
#ifdef EC_NOKILL
        // mandatory additional credit code
        alarm(timeLimit);
#endif
        char *const envp[] = {NULL}; // according to ed #88
        execve(argsBuffer[0], argsBuffer, envp);
        perror(argsBuffer[0]); // refer to TA's demo
        free(userInput); // free it in the child process
        exit(EXIT_FAILURE);
    }

    // parent process
    if (childPid > 0) {
        alarm(timeLimit);
        do {
            if (wait(&wstatus) == -1) {
                perror("Failed to wait.");
                free(userInput);
                exit(EXIT_FAILURE);
            }
        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
    }

    childPid = -1; // reset the child pid
    alarm(0); // reset the alarm
}

void setSignalHandler() {
    if (signal(SIGALRM, SIGALRMHandler) == SIG_ERR) {
        perror("Failed to set SIGALRM handler.");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, SIGINTHandler) == SIG_ERR) {
        perror("Failed to set SIGINT handler.");
        exit(EXIT_FAILURE);
    }
}

void SIGALRMHandler(int sig) {
    if (childPid > 0) {
        writeCatchphrase();
    }

#ifdef EC_NOKILL
    // mandatory additional credit code
    if (childPid == 0) {
        exit(0);
    }
#else
    // regular credit code
    if (childPid != -1 && kill(childPid, SIGKILL) == -1) {
        perror("Failed to kill the child process.");
        exit(EXIT_FAILURE);
    }
#endif
}

void SIGINTHandler(int sig) {
    writeNewline();

    if (childPid == -1) {
        writePrompt();
    }

#ifdef EC_NOKILL
    // mandatory additional credit code
    if (childPid == 0) {
        exit(0);
    }
#else
    // regular credit code
    if (childPid != -1 && kill(childPid, SIGKILL) == -1) {
        perror("Failed to kill the child process.");
        exit(EXIT_FAILURE);
    }
#endif
}