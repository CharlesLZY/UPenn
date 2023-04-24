#include "utils.h"

void writeNewline() {
    if (write(STDERR_FILENO, "\n", 1) == -1) {
        perror("Failed to write the newline.");
        exit(EXIT_FAILURE);
    }
}