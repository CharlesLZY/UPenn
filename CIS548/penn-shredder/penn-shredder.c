#include "shredder-behavior.h"

int main(int argc, char **argv) {
    int timeLimit = parseArgument(argc, argv);

    setSignalHandler();

    char inputBuffer[MAX_LINE_LENGTH];
    char *argsBuffer[MAX_ARGS_NUM] = {NULL};
    while (1) {
        writePrompt();

        char *userInput = readInput(inputBuffer);

        int parseResult = parseInput(userInput, argsBuffer);

        if (parseResult == EXIT_SHREDDER) {
            break;
        }
        else if (parseResult == EMPTY_LINE) {
            writeNewline();
        }
        else {
            executeCommand(userInput, argsBuffer, timeLimit);
        }
        free(userInput);
    }

    return 0;
}