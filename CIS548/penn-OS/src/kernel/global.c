#include "global.h"

// global variables

pcb* active_process;

priority_queue* ready_queue;

// easier for p_waitpid()
pcb_queue* exited_queue;

// TODO: changed the naming to blocked queue instead
pcb_queue* stopped_queue;

ucontext_t main_context;

ucontext_t scheduler_context;

ucontext_t* p_active_context;

pid_t lastPID;

bool stopped_by_timer;

// increments once every quanta
int tick_tracker;