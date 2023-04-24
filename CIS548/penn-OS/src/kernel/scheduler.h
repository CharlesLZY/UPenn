#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <signal.h>    // sigaction, sigemptyset, sigfillset, signal
#include <stdbool.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <sys/time.h>  // setitimer
#include <ucontext.h>  // getcontext, makecontext, setcontext, swapcontext    
#include <valgrind/valgrind.h> 

#include "utils.h"
#include "global.h"
#include "kernel.h"
#include "log.h"

extern pcb* active_process;
extern priority_queue* ready_queue;
extern ucontext_t main_context;
extern ucontext_t scheduler_context;
extern ucontext_t* p_active_context;
extern bool stopped_by_timer;
extern pcb_queue* exited_queue;
extern pcb_queue* stopped_queue;
extern int tick_tracker;

#define TICK 100000     // 1 tick = 0.1s

/**
 * @brief Set the alarm handler object
 * 
 * @return int 
 */
int set_alarm_handler();    

/**
 * @brief The signal handler for SIGALARM
 * 
 */
void alarm_handler();

/**
 * @brief set up time interval for SIGALARM
 * 
 * @return int 
 */
int set_timer();  

/**
 * @brief get the next process to run from the ready queue based on priority
 * 
 * @return pcb* 
 */
pcb* next_process(); 

/**
 * @brief Get the node with pid from all the queues
 * 
 * @param pid 
 * @return pcb_node* 
 */
pcb_node* get_node_by_pid_all_queues(pid_t pid);

/**
 * @brief The function of scheduler
 * 
 */
void scheduler();   

/**
 * @brief set up the idle process
 * 
 */
void idle_func();   

/**
 * @brief suspend the idle process until a signal is delivered to it
 * 
 * @return int 
 */
int scheduler_init();   

/**
 * @brief initialize the idle process
 * 
 * @return int 
 */
int idle_process_init();    // initialize the idle process

/**
 * @brief check if the process has children to wait on
 * 
 * @param process 
 * @return int 
 */
int haveChildrenToWait(pcb *process);

/**
 * @brief deconstruct the idle process
 * 
 */
void deconstruct_idle();

#endif