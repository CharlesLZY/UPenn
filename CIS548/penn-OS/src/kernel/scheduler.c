#include "scheduler.h"

static pcb* idle_process;

int set_alarm_handler() {
    struct sigaction act;

    act.sa_handler = alarm_handler;
    act.sa_flags = SA_RESTART;
    sigfillset(&act.sa_mask);

    if (sigaction(SIGALRM, &act, NULL) == -1) {
        perror("Error in set_alarm_handler.\n");
        return -1;
    }
    return 0;
}


void alarm_handler(int signum)
{
    stopped_by_timer = true;
    tick_tracker++;

    // check all processes in the block queue, if sleep has finished
    pcb_node* n = stopped_queue->head;
    while (n != NULL) {
        pcb_node* tmp = n;
        n = n->next;
        // sleep finished
        if (strcmp(tmp->pcb->pname, "sleep") == 0) {
            if (tmp->pcb->state != STOPPED) {
                if (tmp->pcb->ticks_left > 0) {
                    tmp->pcb->ticks_left--;
                } else{
                    tmp->pcb->state = RUNNING;
                    // add to ready queue
                    pcb_node* p_node = dequeue_by_pid(stopped_queue, tmp->pcb->pid);
                    enqueue_by_priority(ready_queue, p_node->pcb->priority, p_node);
                }
            }
            
        }
    }
    swapcontext(&active_process->ucontext, &scheduler_context);
}


int set_timer(void)
{
    struct itimerval it;

    it.it_interval = (struct timeval){.tv_usec = TICK};
    it.it_value = it.it_interval;

    // schedule a SIGALRM signal to be delivered
    if (setitimer(ITIMER_REAL, &it, NULL) == -1) {
        return FAILURE;
    }
    return SUCCESS;
}


pcb* next_process() {
    // printf("choosing the next process\n");
    if (is_priority_queue_empty(ready_queue)) {
        return idle_process;
    }

    pcb_queue* chosen_queue = NULL;
    while(true) {
        int proposal = pick_priority();
        // printf("priority in proposal is: %i\n", proposal);
        if (proposal == HIGH && !is_empty(ready_queue->high)) {
            chosen_queue = ready_queue->high;
        } else if (proposal == MID && !is_empty(ready_queue->mid)) {
            chosen_queue = ready_queue->mid;
        } else if (proposal == LOW && !is_empty(ready_queue->low)) { 
            chosen_queue = ready_queue->low;
        }

        if (chosen_queue == NULL) {
            continue;
        }

        // printf("next process to run is: %i\n", chosen_queue->head->pcb->pid);
        return chosen_queue->head->pcb;
    }
}


void scheduler() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    set_timer();

    // clean up the previous process
    // make sure the current context is not the scheduler context and ready queue is not empty
    
    if (p_active_context != NULL && active_process->state != BLOCKED && memcmp(p_active_context, &scheduler_context, sizeof(ucontext_t)) != 0 && active_process != idle_process) {
      
        // first remove it from the ready queue
        pcb_node *currNode = dequeue_front_by_priority(ready_queue, active_process->priority);
        
        if (currNode != NULL) {
            log_event(active_process, "DQ_READY_SCHD");
            if (active_process->pid != currNode->pcb->pid) {
                printf("Error: active process is not the head of the ready queue\n");
            }

            // check how the previous process ended
            if (stopped_by_timer || active_process->pid == 1) {
                
                active_process->prev_state = READY;
                active_process->state = READY;

                // since the process hasn't completed yet, we add it back to the ready queue
                log_event(active_process, "EQ_READY_TOUT");
                enqueue_by_priority(ready_queue, active_process->priority, currNode);
                stopped_by_timer = false;
            } 
            else {
                log_event(active_process, "FINISHED");

                if (active_process->state == RUNNING) {
                        
                    // process completed, add it to the exit queue
                    
                    log_event(currNode->pcb, "EXITED");
                    enqueue(exited_queue, currNode);
    
                    active_process->prev_state = active_process->state;
                    active_process->state = EXITED;

                    //printf("process is finished (not stopped by the timer)\n");
                    pcb_node* parent = get_node_by_pid_all_queues(active_process->ppid);
                    if (parent != NULL) {
                        log_event(active_process, "ZOMBIE");
                        // remove the node from children queue and add it to the zombies queue
                        pcb_node *newZombie = dequeue_by_pid(parent->pcb->children, active_process->pid);
                        newZombie->pcb->toWait = false;
                        // printf("move node to zombies\n");
                        enqueue(parent->pcb->zombies, newZombie);

                        // if the parent is blocked waiting for it, unblock the parent
                        // printf("unblocking the parent: %i\n", parent->pcb->pid);
                        if (parent->pcb->ticks_left == -1) {
                            pcb_node* parent = get_node_by_pid(stopped_queue, active_process->ppid);
                            parent->pcb->ticks_left = 0;
                            process_unblock(active_process->ppid);
                        }

                    } else {
                        printf("Active process's pid %d ppid %d\n", active_process->pid, active_process->ppid);
                        printf("Parent node is not supposed to be null\n");
                    }
                    
                    // orphan clean ups
                    if (clean_orphan(active_process) == FAILURE) {
                        perror("Failed to cleanup zombies.\n");
                    }
                    
                } else {
                    log_event(active_process, "DONTBEHERE");
                    printf("Should not enter here\n");
                }
            }   
        }
    }

    active_process = next_process();
    p_active_context = &active_process->ucontext;
    active_process->state = RUNNING;
    stopped_by_timer = false;

    log_event(active_process, "SCHEDULE");
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    if (active_process == idle_process) {
        // p_active_context = &active_process->ucontext;
        setcontext(p_active_context);
    } else {
        // p_active_context = &active_process->ucontext;
        setcontext(p_active_context);
    }
}


int scheduler_init() {

    printf("Initializing scheduler\n");

    // initialize context for the scheduler 
    if (makeContext(&scheduler_context, scheduler, 0, NULL, NULL) == FAILURE) {
        return FAILURE;
    }

    printf("initialized context for the scheduler \n");

    // catch the SIGALRM signal
    if (set_alarm_handler() == FAILURE) {
        return FAILURE;
    }

    printf("set the SIGALRM signal handler\n");

    // schedule a SIGALRM signal to your OS every 100 milliseconds
    if (set_timer() == FAILURE) {
        return FAILURE;
    }

    printf("set the timer\n");

    // initialize the idle process
    if (idle_process_init() == FAILURE) {
        return FAILURE;
    }

    printf("initialized idle process\n");

    return SUCCESS;
}


// suspend the idle process until a signal is delivered to it
void idle_func() {
    sigset_t mask;
    // initializes the signal mask to be an empty set
    if (sigemptyset(&mask) == -1) {
        perror("Failed to initialize signal mask in idle_func");
        return;
    }
    while (true) {
        sigsuspend(&mask);
    }
}


int idle_process_init() {
    //malloc space and initialize attributes
    idle_process = malloc(sizeof(pcb));
    if (makeContext(&idle_process->ucontext, idle_func, 0, NULL, NULL) == -1) {
        return FAILURE;
    }
    idle_process->pid = 0;
    idle_process->ppid = 0;
    idle_process->state = READY;
    idle_process->pname = "idle";
    // strcpy(idle_process->pname, "idle");

    return SUCCESS;
}

pcb_node* get_node_by_pid_all_queues(pid_t pid) {
    pcb_node* ready_node = get_node_from_ready_queue(ready_queue, pid);
    if (ready_node == NULL) {
        pcb_node* stop_node = get_node_by_pid(stopped_queue, pid);
        if (stop_node == NULL) {
            pcb_node* exit_node = get_node_by_pid(exited_queue, pid);
            return exit_node;
        } else {
            return stop_node;
        }
    } else {
        return ready_node;
    }
}


int haveChildrenToWait(pcb *process) {
    if (is_empty(process->children)) {
        return 0;
    } else {
        pcb_node *node = process->children->head;
        while (node) {
            if (node->pcb->toWait) {
                return 1;
            }
            node = node->next;
        }
        return 0;
    }
}


void deconstruct_idle() {
    free(idle_process->ucontext.uc_stack.ss_sp);
    free(idle_process);
}
