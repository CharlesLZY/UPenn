#ifndef KERNEL_H
#define KERNEL_H

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "utils.h"
#include "global.h"
#include "job.h"
#include "log.h"

extern ucontext_t scheduler_context;
extern pid_t lastPID;
extern priority_queue* ready_queue;
extern bool stopped_by_timer;
extern pcb_queue* exited_queue;
extern pcb_queue* stopped_queue;

extern JobList _jobList;
extern pid_t fgPid;

/**
 * @brief create a new child thread and associated PCB. The new thread should retain much of the properties of the parent. 
        The function should return a reference to the new PCB.
 * 
 * @param parent 
 * @return pcb* 
 */
pcb *k_process_create(pcb * parent);

/**
 * @brief handle the S_SIGSTOP, S_SIGCONT, S_SIGTERM signals sent to the process
 * 
 * @param process 
 * @param signal 
 * @return int 
 */
int k_process_kill(pcb *process, int signal);

/**
 * @brief called when a terminated/finished thread’s resources needs to be cleaned up. 
        Such clean-up may include freeing memory, setting the status of the child, etc.
 * 
 * @param process 
 * @return int 
 */
int k_process_cleanup(pcb *process);

/**
 * @brief initialize kernel 
 * 
 * @return int 
 */
int kernel_init();

/**
 * @brief deconstruct kernel
 * 
 */
void kernel_deconstruct();

/**
 * @brief block a process, add the process to stopped queue
 * 
 * @param pid 
 * @return int 
 */
int block_process(pid_t pid);

/**
 * @brief unblock a process. Remove it from the stopped queue and add it back to the ready queue
 * 
 * @param pid 
 * @return int 
 */
int process_unblock(pid_t pid);

/**
 * @brief clean up the orphan processes
 * 
 * @param process 
 * @return int 
 */
int clean_orphan(pcb * process);

#endif