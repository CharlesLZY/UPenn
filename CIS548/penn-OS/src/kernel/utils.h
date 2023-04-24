#ifndef UTILS_H
#define UTILS_H

#include <signal.h>    // sigaction, sigemptyset, sigfillset, signal
#include <stdbool.h>
#include <stdio.h>     
#include <string.h>
#include <stdlib.h>    
#include <sys/time.h>  // setitimer
#include <ucontext.h>  // getcontext, makecontext, setcontext, swapcontext    
#include <time.h>
#include <valgrind/valgrind.h> 
#include "global2.h"
#include "perrno.h"
#include "../PennFAT/fd-table.h"

/**
 * @brief attributes of PCB
 * 
 */
typedef struct pcb {
    ucontext_t ucontext;
    pid_t pid;
    pid_t ppid;     //!< parent pid
    enum process_state prev_state;
    enum process_state state;      //!< state of the process
    int priority;   //!< priority of the process
    FdNode *fds[MAX_FILE_DESCRIPTOR];   //!< keep track of open FDs
    int ticks_left;     //!< ticks left for sleep to be blocked
    struct pcb_queue* children;     //!< processes that have not completed yet
    struct pcb_queue* zombies;      //!< processes that are completed but the parent has not waited for it yet
    char* pname;    //!< name of the function 
    bool toWait;    //!< true indicates the parent need to wait for this child
} pcb;

/**
 * @brief node in the pcb_queue
 * 
 */
typedef struct pcb_node {
    pcb* pcb;
    struct pcb_node* next;
} pcb_node;

/**
 * @brief A link list of pcb nodes
 * 
 */
typedef struct pcb_queue {
    pcb_node* head;
    pcb_node* tail;
} pcb_queue;

/**
 * @brief the ready queue including three pcb queues of different priorities
 * 
 */
typedef struct priority_queue {  
    pcb_queue* high;    //!< priority -1
    pcb_queue* mid;     //!< priority 0
    pcb_queue* low;     //!< priority 1
} priority_queue;

/**
 * @brief create a new pcb with the given ucontext
 * 
 * @param ucontext 
 * @param pid 
 * @return pcb* 
 */
pcb* new_pcb(ucontext_t* ucontext, pid_t pid);

/**
 * @brief Create a new pcb node with the given pcb
 * 
 * @param pcb 
 * @return pcb_node* 
 */
pcb_node* new_pcb_node(pcb* pcb);       

/**
 * @brief Create an empty pcb queue
 * 
 * @return pcb_queue* 
 */
pcb_queue* new_pcb_queue();    

/**
 * @brief Create an empty priority queue
 * 
 * @return priority_queue* 
 */
priority_queue* new_priority_queue();

/**
 * @brief Check if the pcb queue is empty
 * 
 * @param queue 
 * @return true 
 * @return false 
 */
bool is_empty(pcb_queue* queue);

/**
 * @brief check if the priority queue is empty
 * 
 * @param ready_queue 
 * @return true 
 * @return false 
 */
bool is_priority_queue_empty(priority_queue* ready_queue);

/**
 * @brief return the pointer to the pcb_queue of the required priority
 * 
 * @param ready_queue 
 * @param priority 
 * @return pcb_queue* 
 */
pcb_queue* get_pcb_queue_by_priority(priority_queue* ready_queue, int priority);

/**
 * @brief Enqueue a new node to the pcb queue
 * 
 * @param queue 
 * @param node 
 */
void enqueue(pcb_queue* queue, pcb_node* node);

/**
 * @brief Enqueue a node to the ready queue based on it's priority
 * 
 * @param ready_queue 
 * @param priority 
 * @param node 
 */
void enqueue_by_priority(priority_queue* ready_queue, int priority, pcb_node* node);

/**
 * @brief Dequeue the node with pid from the queue
 * 
 * @param queue 
 * @param pid 
 * @return pcb_node* 
 */
pcb_node* dequeue_by_pid(pcb_queue* queue, pid_t pid);

/**
 * @brief Dequeue the first node from the queue
 * 
 * @param queue 
 * @return pcb_node* 
 */
pcb_node *dequeue_front(pcb_queue* queue);

/**
 * @brief  Dequeue the first node from the queue based on the priority
 * 
 * @param ready_queue 
 * @param priority 
 * @return pcb_node* 
 */
pcb_node *dequeue_front_by_priority(priority_queue* ready_queue, int priority); 

/**
 * @brief Get the node by pid from the queue
 * 
 * @param queue 
 * @param pid 
 * @return pcb_node* 
 */
pcb_node* get_node_by_pid(pcb_queue* queue, pid_t pid);

/**
 * @brief Get the node by pid from the priority queue
 * 
 * @param ready_queue 
 * @param pid 
 * @return pcb_node* 
 */
pcb_node* get_node_by_pid_from_priority_queue(priority_queue* ready_queue, pid_t pid);

/**
 * @brief free the pcb queue
 * 
 * @param queue 
 */
void deconstruct_queue(pcb_queue* queue);

/**
 * @brief free the pcb priority queue
 * 
 * @param ready_queue 
 */
void deconstruct_priority_queue(priority_queue* ready_queue);

/**
 * @brief Get the node with pid from ready queue
 * 
 * @param ready_queue 
 * @param pid 
 * @return pcb_node* 
 */
pcb_node* get_node_from_ready_queue(priority_queue* ready_queue, pid_t pid);

/**
 * @brief Randomly pick a queue from ready queue based on the priority
 * 
 * @return int 
 */
int pick_priority();

/**
 * @brief initialize stack for ucontext
 * 
 * @param stack 
 */
void set_stack(stack_t *stack);

/**
 * @brief initializing context
 * 
 * @param ucp 
 * @param func 
 * @param argc 
 * @param next_context 
 * @param argv 
 * @return int 
 */
int makeContext(ucontext_t *ucp,  void (*func)(), int argc, ucontext_t *next_context, char *argv[]);

/**
 * @brief print out the queue for ps
 * 
 * @param queue 
 * @return int 
 */
int printQueue(pcb_queue *queue);

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_FINISHED,
    JOB_TERMINATED
} JobState;


typedef enum {
    NICE, // Syntax: nice priority command [args]
    NICE_PID, // Syntax: nice_pid priority pid
    MAN, // Syntax: man
    BG, // Syntax: bg [job_id]
    FG, // Syntax: fg [job_id]
    JOBS, // Syntax: jobs
    LOGOUT, // Syntax: logout
    KILL,
    OTHERS // non-builtin command
} CommandType;

typedef struct Job {
    struct parsed_command *cmd;
    pid_t pid;
    JobState state;
} Job;

typedef struct JobListNode {
    Job *job;
    struct JobListNode *prev;
    struct JobListNode *next;
    int jobId;
} JobListNode;

typedef struct JobList {
    JobListNode *head;
    JobListNode *tail;
    int jobCount;
} JobList;

pcb_queue* sortQueue(pcb_queue* queue);

pcb_queue* merge_two_queues(pcb_queue* queue1, pcb_queue* queue2);

extern priority_queue* ready_queue;
extern pcb_queue* exited_queue;
extern pcb_queue* stopped_queue;

extern pcb_node* get_node_by_pid_all_queues(pid_t pid);

#endif