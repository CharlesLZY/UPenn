#include "log.h"
FILE *log_file;

int log_init(const char *filename) {
    if (filename == NULL) {
        log_file = fopen(LOG_FILE, "w");
        if (log_file == NULL) {
            printf("Error opening file at %s\n", LOG_FILE);
            return FAILURE;
        }
    } else {
        log_file = fopen(filename, "w");
        if (log_file == NULL) {
            printf("Error opening file at %s\n", filename);
            return FAILURE;
        }
    }
    return SUCCESS;
}

void log_event(pcb* pcb, char* action) {
    fprintf(log_file, "[%4d] %14s %2d %3d %6s\n", tick_tracker, action, pcb->pid, pcb->priority, pcb->pname);
    fflush(log_file);
}

void log_pnice(pcb* pcb, int new) {
    // [ticks] NICE PID OLD_NICE_VALUE NEW_NICE_VALUE PROCESS_NAME
    fprintf(log_file, "[%4d] %14s %2d %3d %3d %6s\n", tick_tracker, "NICE", pcb->pid, pcb->priority, new, pcb->pname);
    fflush(log_file);
}

void log_cleanup() {
    fclose(log_file);
}