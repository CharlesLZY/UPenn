#include "parser.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    struct parsed_command *cmd;

    int i = parse_command("cat < asd |  sleep >>test.c 1 & ", &cmd);
    if (i < 0) {
        perror("parse_command");
        return 1;
    }
    if (i > 0) {
        printf("parser error: %d\n", i);
        return 1;
    }

    print_parsed_command(cmd);

    printf("Is background job:\t%s\n", cmd->is_background ? "true" : "false");
    printf("Is append output file:\t%s\n", cmd->is_file_append ? "true" : "false");

    printf("Input file name:\t%s\n", cmd->stdin_file);
    printf("Output file name:\t%s\n", cmd->stdout_file);

    printf("Number of commands:\t%zu\n", cmd->num_commands);

    for (int i = 0; i < cmd->num_commands; ++i) {
        printf("Pipeline %d:\t%p\n", i, cmd->commands[i]);
        // directly use commands[i] as argv, e.g.
        //   execvp(cmd->commands[i][0], cmd->commands[i]);
    }

    free(cmd);
    return 0;
}