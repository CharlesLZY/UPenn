#ifndef UTILS_H
#define UTILS_H

#include <unistd.h> // system calls
#include <sys/wait.h> // wait
#include <signal.h> // signal, kill
#include <fcntl.h> // open, close
#include <stdlib.h> // malloc, free, atoi
#include <string.h> // strlen, strtok
#include <stdio.h> // perror
#include <stdbool.h> // bool

#include "parser.h" // sophisticated command parser provided by the TAs

void writeNewline();

// #define DEBUG_INFO
// #define ENABLE_KILL

#endif