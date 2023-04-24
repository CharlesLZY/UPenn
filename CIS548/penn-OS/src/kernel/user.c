#include "user.h"

bool W_WIFEXITED(int status) {
    return status == EXITED;
}

bool W_WIFSTOPPED(int status) {
    return status == STOPPED;
}

bool W_WIFSIGNALED(int status) {
    return status == TERMINATED;
}


pid_t p_spawn(void (*func)(), char *argv[], int fd0, int fd1) {
    // forks a new thread that retains most of the attributes of the parent thread 
    pcb* pcb = k_process_create(active_process);
    if (pcb == NULL) {
        p_perror("failed to create pcb");
        return -1;
    }

    // copy parent's fd table
    for (int fd_idx = 0; fd_idx < MAX_FILE_DESCRIPTOR; fd_idx ++) {
        FdNode *parent_fnode = (active_process->fds)[fd_idx];
        (pcb->fds)[fd_idx] = parent_fnode;
    }
    
    // redirection of file descriptors
    if (fd0 != F_STDIN_FD) {
        FdNode *dst_node = pcb->fds[fd0];
        if (dst_node == NULL) {
            printf("[Error] Input file not opened\n");
            return FAILURE;
        }
        
        pcb->fds[F_STDIN_FD] = dst_node;
    }
    
    if (fd1 != F_STDOUT_FD) {
        FdNode *dst_node = pcb->fds[fd1];
        if (dst_node == NULL) {
            printf("[Error] Output file not opened\n");
            return FAILURE;
        }
        
        pcb->fds[F_STDOUT_FD] = dst_node;
    }

    // we might not need this:
    int num_args = 0;
    while (argv[num_args] != NULL) {
        num_args++;
    }

    // executes the function referenced by func with its argument array argv
    makeContext(&(pcb->ucontext), func, num_args, &scheduler_context, argv);

    // assign process name
    pcb->pname = malloc(sizeof(char) * (strlen(argv[0]) + 1));
    strcpy(pcb->pname, argv[0]);    // the first arg is the name of the func

    pcb_node* newNode1 = new_pcb_node(pcb);
    // default priority level is 0
    // printf("spawned. add to ready queue: %d\n", newNode->pcb->pid);
    enqueue(ready_queue->mid, newNode1);
    // add to the children list for the parent
    pcb_node* newNode2 = new_pcb_node(pcb);
    enqueue(active_process->children, newNode2);
    log_event(pcb, "CREATE");
    return pcb->pid;
}

void cleanup(pcb_queue* queue, pcb_node* child) {
    if (!is_empty(queue) && child != NULL) {
        if (child->pcb->state == TERMINATED) {
            // clean up the child process
            dequeue_by_pid(queue, child->pcb->pid);
            //k_process_cleanup(child->pcb);
        }
    } else {
        printf("parent queue and child are both not supposed to be null\n");
    }
}

pid_t wait_for_one(pid_t pid, int *wstatus) {
    pcb* parent = active_process; // the calling thread
    
    pcb_node* child = get_node_by_pid_all_alive_queues(pid); // ready & stopped queue
    if (child == NULL) {
        child = get_node_by_pid(exited_queue, pid);
    }

    // reap zombie
    pcb_node* zombie = get_node_by_pid(parent->zombies, pid);
    if (zombie != NULL) {
        log_event(zombie->pcb, "WAITED");
        dequeue_by_pid(parent->zombies, pid);
        dequeue_by_pid(exited_queue, pid);
        free(zombie->pcb);
    }

    if (child == NULL) {
        printf("Error: cannot find a process with this pid: %d\n", pid);
        return -1;
    }

    if (child->pcb->ppid != parent->pid) {
        printf("Error: the calling thread is not the process's parent pid: %d\n", pid);
        return -1;
    }

    if (child->pcb->prev_state != child->pcb->state && !(child->pcb->prev_state == RUNNING && child->pcb->state == READY)) {
        child->pcb->prev_state = child->pcb->state;
        log_event(child->pcb, "WAITED");
        if (wstatus != NULL) {
            *wstatus = child->pcb->state;
        }
        return pid;
    }

    // if WNOHANG was specified and one or more child(ren) specified by pid exist, 
    // but have not yet changed state, then 0 is returned.
    return 0;
}

pid_t wait_for_anyone(int *wstatus) {
    // if zombie queue is not empty, then we return the first zombie
    if (!is_empty(active_process->zombies)) {
        pcb_node* zombie_node = active_process->zombies->head;
        log_event(zombie_node->pcb, "WAITED");
        pid_t zombiePID = zombie_node->pcb->pid;
        // set the status
        if (wstatus != NULL) {
            *wstatus = zombie_node->pcb->state;
        }
        dequeue_by_pid(active_process->zombies, zombiePID);
        dequeue_by_pid(exited_queue, zombiePID);
        free(zombie_node->pcb);
        return zombiePID;
    }

    // then we traverse through the children 
    pcb_queue* children = active_process->children;
    if (!is_empty(children)) {
        for (pcb_node* child = children->head; child != NULL; child = child->next) {
            if (child->pcb->prev_state != child->pcb->state && !(child->pcb->prev_state == RUNNING && child->pcb->state == READY)) {
                log_event(child->pcb, "WAITED");
                child->pcb->prev_state = child->pcb->state;
                // set the status
                if (wstatus != NULL) {
                    *wstatus = child->pcb->state;
                }
                return child->pcb->pid;
            }
        }
    }
    return 0;
}

pid_t p_waitpid(pid_t pid, int *wstatus, bool nohang) {
    // printf("waitpid is called withh pid: %d\n", pid);
    // printf("waitpid is called withh active process: %d\n", active_process->pid);
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    
    // if there is no children to wait for, return -1
    if (is_empty(active_process->children) && is_empty(active_process->zombies)) {
        // printf("shouldnt be here\n");
        return -1;
    }

    if (pid > 0) {  // a particule process

        if (nohang) {   
            // non blocking
            return wait_for_one(pid, wstatus);
        } else {
            // if there are zombies, reap and return right away
            // sigprocmask(SIG_BLOCK, &mask, NULL);
            pid_t result = wait_for_one(pid, wstatus);
            if (result != 0) {
                return result;
            }

            // block the calling thread
            // TODO: still dk how this will block the process
            // this is how children would know parent is waiting
            active_process->ticks_left = -1; 
            // pcb_node *child = get_node_by_pid(active_process->children, pid);
            // child->pcb->toWait = true;

            block_process(active_process->pid);
            // switch context to scheduler 
            stopped_by_timer = false;

            // sigprocmask(SIG_UNBLOCK, &mask, NULL);
            swapcontext(&active_process->ucontext, &scheduler_context);

            // at this point, the parent process should be unblocked
            result = wait_for_one(pid, wstatus);

            if (result == 0) {
                printf("cannot 0, should return pid instead because nohang is false\n");
                return -1;
            }
            return result;
        }
    } else {
        if (pid == -1) {
            if (nohang) {
                return wait_for_anyone(wstatus);
            } else {
                sigprocmask(SIG_BLOCK, &mask, NULL);
                log_event(active_process, "WAIT_1");
                pid_t result = wait_for_anyone(wstatus);
                // printf("wait for anyone result 1: %d\n", result);
                if (result != 0) {
                    return result;
                }

                // block parent, remove it from the ready queue and switch context
                active_process->ticks_left = -1; 

                block_process(active_process->pid);
                sigprocmask(SIG_UNBLOCK, &mask, NULL);

                // switch context to scheduler 
                stopped_by_timer = false;

                swapcontext(&active_process->ucontext, &scheduler_context);

                log_event(active_process, "WAIT_2");
                result = wait_for_anyone(wstatus);
                // printf("wait for anyone result 2: %d\n", result);

                if (result == 0) {
                    printf("cannot 0, should return pid instead because nohang is false\n");
                    return -1;
                }
                return result;
            }
        } else {
            printf("the pid is not greater than 0 or -1, error!\n");
            return -1;
        }
    }
    return -1;
}

int p_kill(pid_t pid, int sig) {
    // TODO: search by pid in all queues
    pcb_node* target_node = get_node_by_pid_all_alive_queues(pid);
    if (target_node == NULL) {
        printf("target node with the pid %i does not exist\n", pid);
        return -1;
    }
    return k_process_kill(target_node->pcb, sig);
}

pcb_node* get_node_by_pid_all_alive_queues(pid_t pid) {
    pcb_node* ready_node = get_node_from_ready_queue(ready_queue, pid);
    if (ready_node == NULL) {
        pcb_node* stop_node = get_node_by_pid(stopped_queue, pid);
        return stop_node;
    } else {
        return ready_node;
    }
}

int p_exit(void) {   
    log_event(active_process, "EXITED");
    if (k_process_cleanup(active_process)) {
        printf("Failed to clean up process.\n");
        return FAILURE;
    }
    // shell
    if (active_process->pid == 1) {
        deconstruct_shell();
    }

    return SUCCESS;
}

int p_nice(pid_t pid, int priority) {
    if (priority < -1 || priority > 1) {
        printf("Priority has to be 1, 0, or -1\n");
        return -1;
    }
    // check if it is in the ready queue
    pcb_node* target_node = get_node_from_ready_queue(ready_queue, pid);
    if (target_node == NULL) {
        // if it is not, just update the priority 
        target_node = get_node_by_pid(stopped_queue, pid);
        if (target_node == NULL) {
            printf("node with the pid %i does not exist\n", pid);
            return -1;
        }
        log_pnice(target_node->pcb, priority);
        target_node->pcb->priority = priority;
    } else {
        log_pnice(target_node->pcb, priority);
        // if it is, change the queue if necessary
        if (target_node->pcb->priority != priority) {
            // change the queue
            pcb_queue* orginal_queue = get_pcb_queue_by_priority(ready_queue, target_node->pcb->priority);
            dequeue_by_pid(orginal_queue, pid);
            target_node->pcb->priority = priority;
            enqueue_by_priority(ready_queue, priority, target_node);
        } 
    }

    return pid;
}

void p_sleep(unsigned int seconds) {
    if (seconds < 1) {
        printf("ticks has to be greater than 1\n");
        return;
    }
    // sets the calling process to blocked until ticks of the system clock elapse
    // and then sets the thread to running 

    active_process->ticks_left = seconds;
    // printf("ticks to reach is %d\n", active_process->ticks_left);

    block_process(active_process->pid);
    p_active_context = NULL;
    swapcontext(&active_process->ucontext, &scheduler_context);
}

void signal_handler(int signal) {
    // if shell, PROMPT again
    if (fgPid == 1) {
        if (signal == SIGINT || signal == SIGTSTP) {
            writePrompt();
        }
    } else {
        if (signal == SIGINT) {
            p_kill(fgPid, S_SIGTERM);
        } else if (signal == SIGTSTP) {
            p_kill(fgPid, S_SIGSTOP);
        }
    }
}

int register_signals() {
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        p_perror("Failed to register handler for SIGINT.\n");
        return FAILURE;
    }

    if (signal(SIGTSTP, signal_handler) == SIG_ERR) {
        p_perror("Failed to register handler for SIGSTOP.\n");
        return FAILURE;
    }

    return SUCCESS;
}

void deconstruct_shell() {

    deconstruct_idle();

    deconstruct_priority_queue(ready_queue);
    deconstruct_queue(stopped_queue);
    deconstruct_queue(exited_queue);

    log_cleanup();
    
    free(main_context.uc_stack.ss_sp);
    free(scheduler_context.uc_stack.ss_sp);

    printf("Log out succeed.\n");

    exit(0);
}