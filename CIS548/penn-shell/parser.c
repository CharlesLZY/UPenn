#include "parser.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>


static inline void skip_word(const char **const cur, const char *const end) {
    while (*cur < end && **cur != '<' && **cur != '>' && **cur != '|' && **cur != '&' && !isspace(**cur)) ++*cur;
}

static inline void skip_space(const char **const cur, const char *const end) {
    while (*cur < end && isspace(**cur)) ++*cur;
}

int parse_command(const char *const cmd_line, struct parsed_command **const result) {
#define JUMP_OUT(code) do {ret_code = code; goto PROCESS_ERROR;} while (0)

    int ret_code = -1;

    const char *start = cmd_line;
    const char *end = cmd_line + strlen(cmd_line);

    for (const char *cur = start; cur < end; ++cur)
        if (*cur == '#') {
            // all subsequent characters following '#'
            // shall be discarded as a comment.
            end = cur;
            break;
        }

    // trimming leading and trailing whitespaces
    while (start < end && isspace(*start)) ++start;
    while (start < end && isspace(end[-1])) --end;

    struct parsed_command *pcmd = calloc(1, sizeof(struct parsed_command));
    if (pcmd == NULL) return -1;
    if (start == end) goto PROCESS_SUCCESS; // empty line, fast pass

    // If a command is terminated by the control operator ampersand ( '&' ),
    // the shell shall execute the command in background.
    if (end[-1] == '&') {
        pcmd->is_background = true;
        --end;
    }

    // first pass, check token
    int total_strings = 0; // number of total arguments
    {
        bool has_token_last = false, has_file_input = false, has_file_output = false;
        const char *skipped;
        for (const char *cur = start; cur < end; skip_space(&cur, end))
            switch (cur[0]) {
                case '&':
                    JUMP_OUT(UNEXPECTED_AMPERSAND); // does not expect anymore ampersand
                case '<':
                    // if already had pipeline or had file input, error
                    if (pcmd->num_commands > 0 || has_file_input) JUMP_OUT(UNEXPECTED_FILE_INPUT);

                    ++cur; // skip '<'
                    skip_space(&cur, end);

                    // test if we indeed have a filename following '<'
                    skipped = cur;
                    skip_word(&skipped, end);
                    if (skipped <= cur) JUMP_OUT(EXPECT_INPUT_FILENAME);

                    // fast-forward to the end of the filename
                    cur = skipped;
                    has_file_input = true;
                    break;
                case '>':
                    // if already had file output, error
                    if (has_file_output) JUMP_OUT(UNEXPECTED_FILE_OUTPUT);
                    if (cur + 1 < end && cur[1] == '>') { // dealing with '>>' append
                        pcmd->is_file_append = true;
                        ++cur;
                    }

                    ++cur; // skip '>'
                    skip_space(&cur, end);

                    // test filename, as the case above
                    skipped = cur;
                    skip_word(&skipped, end);
                    if (skipped <= cur) JUMP_OUT(EXPECT_OUTPUT_FILENAME);

                    // fast-forward to the end of the filename
                    cur = skipped;
                    has_file_output = true;
                    break;
                case '|':
                    // if already had file output but encourter a pipeline, it should
                    // rather be a file output error instead of a pipeline one.
                    if (has_file_output) JUMP_OUT(UNEXPECTED_FILE_OUTPUT);
                    // if no tokens between two pipelines (or before the first one)
                    // should throw a pipeline error
                    if (!has_token_last) JUMP_OUT(UNEXPECTED_PIPELINE);
                    has_token_last = false;
                    ++pcmd->num_commands;
                    ++cur; // skip '|'
                    break;
                default:
                    has_token_last = true;
                    ++total_strings;
                    skip_word(&cur, end); // skip that argument
            }

        if (total_strings == 0) {
            // if there are no arguments but has ampersand or file input/output
            // then we have an error
            if (pcmd->is_background || has_file_input || has_file_output)
                JUMP_OUT(EXPECT_COMMANDS);
            // otherwise it's an empty line
            goto PROCESS_SUCCESS;
        }

        // handle edge case where the command ends with a pipeline
        // (not supporting line continuation)
        if (!has_token_last) JUMP_OUT(UNEXPECTED_PIPELINE);
    }
    ++pcmd->num_commands;

    /** layout of memory for `struct parsed_command`
        bool is_background;
        bool is_file_append;

        const char *stdin_file;
        const char *stdout_file;

        size_t num_commands;

        // commands are pointers to `arguments`
        char **commands[num_commands];

        ** below are hidden in memory **

        // arguments are pointers to `original_string`
        // `+ num_commands` because all argv are null-terminated
        char *arguments[total_strings + num_commands];

        // original_string is a copy of the cmdline
        // but with each token null-terminated
        char *original_string;
    */

    const size_t start_of_array = offsetof(struct parsed_command, commands) +pcmd->num_commands * sizeof(char **);
    const size_t start_of_str = start_of_array + (pcmd->num_commands + total_strings) * sizeof(char *);
    const size_t slen = end - start;

    char *const new_buf = realloc(pcmd, start_of_str + slen + 1);
    if (new_buf == NULL) goto PROCESS_ERROR;
    pcmd = (struct parsed_command *) new_buf;

    // copy string to the new place
    char *const new_start = memcpy(new_buf + start_of_str, start, slen);

    // second pass, put stuff in
    // no need to check for error anymore
    size_t cur_cmd = 0;
    char **argv_ptr = (char **) (new_buf + start_of_array);

    pcmd->commands[cur_cmd] = argv_ptr;
    for (const char *cur = start; cur < end; skip_space(&cur, end)) {
        switch (cur[0]) {
            case '<':
                ++cur;
                skip_space(&cur, end);
                // store input file name into `stdin_file`
                pcmd->stdin_file = new_start + (cur - start);
                skip_word(&cur, end);
                // at end of the input file name
                new_start[cur - start] = '\0';
                break;
            case '>':
                if (pcmd->is_file_append) ++cur; // skip another '>'
                ++cur;
                skip_space(&cur, end);
                // store output file name into `stdout_file`
                pcmd->stdout_file = new_start + (cur - start);
                skip_word(&cur, end);
                // at end of the output file name
                new_start[cur - start] = '\0';
                break;
            case '|':
                // null-terminate the current argv
                *(argv_ptr++) = NULL;
                // store the next argv head
                pcmd->commands[++cur_cmd] = argv_ptr;
                ++cur;
                break;
            default:
                // at start of the argument string
                // store it into the arguments array
                *(argv_ptr++) = new_start + (cur - start);
                skip_word(&cur, end);
                // at end of the argument string
                new_start[cur - start] = '\0';
        }
    }
    // null-terminate the last argv
    *argv_ptr = NULL;

PROCESS_SUCCESS:
    *result = pcmd;
    return 0;
PROCESS_ERROR:
    free(pcmd);
    return ret_code;
}

#include <stdio.h>

void print_parsed_command(const struct parsed_command *const cmd) {
    for (size_t i = 0; i < cmd->num_commands; ++i) {
        for (char **arguments = cmd->commands[i]; *arguments != NULL; ++arguments)
            printf("%s ", *arguments);

        if (i == 0 && cmd->stdin_file != NULL)
            printf("< %s ", cmd->stdin_file);

        if (i == cmd->num_commands - 1) {
            if (cmd->stdout_file != NULL)
                printf(cmd->is_file_append ? ">> %s " : "> %s ", cmd->stdout_file);
        } else printf("| ");
    }
    puts(cmd->is_background ? "&" : "");
}
