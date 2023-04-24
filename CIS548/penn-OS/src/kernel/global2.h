#ifndef GLOBAL2_H
#define GLOBAL2_H

/**
 * state of the process
 */ 
typedef enum process_state {
    RUNNING,
    READY,
    BLOCKED,
    STOPPED,
    ZOMBIED,
    ORPHANED,
    TERMINATED,     // terminated by signal
    EXITED,         // terminated normally
} process_state;

#endif