#include "kernel.h"
#include "utils.h"
#include "global.h"
#include "job.h"
#include "log.h"

/**
 * @brief user level functions
 * @file user.h
 */

extern priority_queue* ready_queue;
extern pcb* active_process;
extern ucontext_t scheduler_context;
extern int tick_tracker;
extern ucontext_t* p_active_context;
extern ucontext_t main_context;

extern JobList _jobList;
extern pid_t fgPid;

extern void writePrompt();
extern void deconstruct_idle();

bool W_WIFEXITED(int status);
bool W_WIFSTOPPED(int status);
bool W_WIFSIGNALED(int status);

void signal_handler(int signal);

int register_signals();

/**
* @brief forks a new thread that retains most of the attributes of the parent thread (see k_process_create).
* @param func the function to be executed by the new thread
* @param argv the arguments to be passed to the function
* @param fd0 the file descriptor to be used as stdin for the new thread
* @param fd1 the file descriptor to be used as stdout for the new thread
* @return the pid of the new thread on success, or -1 on error
**/
pid_t p_spawn(void (*func)(), char *argv[], int fd0, int fd1);

// helper functions for waitpid
pid_t wait_for_one(pid_t pid, int *wstatus);
pid_t wait_for_anyone(int *wstatus);

/**
 * @brief sets the calling thread as blocked (if nohang is false) until a child of the calling thread changes state
 * @param pid pid of the child thread to wait for
 * @param wstatus status of the child thread
 * @param nohang whether to block the calling thread or not
 * @return pid_t 
 */
pid_t p_waitpid(pid_t pid, int *wstatus, bool nohang);

/**
 * @brief Get the node by pid all alive queues object
 * @param pid 
 * @return pcb_node* 
 */
pcb_node* get_node_by_pid_all_alive_queues(pid_t pid);     

/**
 * @brief sends the signal sig to the thread referenced by pid.
 * @param pid pid of the thread to send the signal to
 * @param sig signal to be sent
 * @return returns 0 on success, or -1 on error. 
 */
int p_kill(pid_t pid, int sig);

/**
 * @brief exits the current thread unconditionally
 * 
 * @return int 
 */
int p_exit(void);

/**
 * @brief sets the priority of the thread pid to priority
 * 
 * @param pid 
 * @param priority 
 * @return int 
 */
int p_nice(pid_t pid, int priority);

/**
 * @brief sleeps the current thread for ticks of the system clock
 * 
 * @param ticks 
 */
void p_sleep(unsigned int ticks);

/**
 * @brief deconstructs the shell
 * 
 */
void deconstruct_shell();