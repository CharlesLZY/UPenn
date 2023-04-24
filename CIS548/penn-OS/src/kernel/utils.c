#include "global.h"
#include "utils.h"

pcb* new_pcb(ucontext_t* ucontext, pid_t pid) {
    pcb* pcb_n = (pcb*)malloc(sizeof(pcb));
    memset(pcb_n, 0, sizeof(pcb));
    pcb_n->ucontext = *ucontext;
    pcb_n->pid = pid;
    pcb_n->children = new_pcb_queue();
    pcb_n->zombies = new_pcb_queue();
    pcb_n->ticks_left = 0;
    pcb_n->priority = 0;
    pcb_n->state = READY;
    pcb_n->pname = "";
    pcb_n->toWait = false;

    for (int i = 0; i < MAX_FILE_DESCRIPTOR; i++) {
        pcb_n->fds[i] = NULL;
    }

    return pcb_n;
}


pcb_node* new_pcb_node(pcb* pcb) {
    pcb_node* node = (pcb_node*)malloc(sizeof(pcb_node));
    node->pcb = pcb;
    node->next = NULL;
    return node;
}

pcb_queue* new_pcb_queue() {
    pcb_queue* queue = (pcb_queue*)malloc(sizeof(pcb_queue));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}


priority_queue* new_priority_queue() {
    priority_queue*  queue = (priority_queue*)malloc(sizeof(priority_queue));
    queue->high = new_pcb_queue();
    queue->mid = new_pcb_queue();
    queue->low = new_pcb_queue();
    return queue;
}


bool is_empty(pcb_queue* queue) {
    return (queue->head == NULL);
}

bool is_priority_queue_empty(priority_queue* ready_queue) {
    return is_empty(ready_queue->high) && is_empty(ready_queue->mid) && is_empty(ready_queue->low);
}

pcb_queue* get_pcb_queue_by_priority(priority_queue* ready_queue, int priority) {
    pcb_queue* queue=NULL;
    if (priority == HIGH) {
        queue = ready_queue->high;
    } else if (priority == MID) {
        queue = ready_queue->mid;
    } else if (priority == LOW) { 
        queue = ready_queue->low;
    }
    return queue;
}

void enqueue(pcb_queue* queue, pcb_node* node) {
    if (is_empty(queue)) {
        queue->head = node;
        queue->tail = node;
        return;
    }
    node->next = NULL;
    queue->tail->next = node;
    queue->tail = node;
}     

void enqueue_by_priority(priority_queue* ready_queue, int priority, pcb_node* node) {
    pcb_queue* queue = get_pcb_queue_by_priority(ready_queue, priority);
    enqueue(queue, node);
}


pcb_node* dequeue_by_pid(pcb_queue* queue, pid_t pid) {
    if (is_empty(queue)) {
        return NULL;
    }
    pcb_node* current = queue->head;
    pcb_node* previous = NULL;
    // Traverse the list until the value is found or the end is reached
    while (current != NULL) {
        if (current->pcb->pid == pid) {
            if (previous == NULL) {
                // is the first element of the queue
                queue->head = current->next;
            } else {
                previous->next = current->next;
            }

            if (current->next == NULL) {
                // is the last element of the queue
                queue->tail = previous;
            }
            break;
        }

        previous = current;
        current = current->next;
    }
    if (current != NULL) {
        current->next = NULL;
    }
    
    return current;
}


pcb_node *dequeue_front(pcb_queue* queue) {
    if (is_empty(queue)) {
        return NULL;
    }

    pcb_node* current = queue->head;

    if (queue->head == queue->tail) {
        // only one node in the queue
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->head = queue->head->next;
    }

    if (current != NULL) {
        current->next = NULL;
    }

    return current;
}


pcb_node *dequeue_front_by_priority(priority_queue* ready_queue, int priority) {
    pcb_queue* queue = get_pcb_queue_by_priority(ready_queue, priority);
    return dequeue_front(queue);
}


pcb_node* get_node_by_pid(pcb_queue* queue, pid_t pid) {
    pcb_node* current = queue->head;
    while (current != NULL) {
        if (current->pcb->pid == pid) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

pcb_node* get_node_by_pid_from_priority_queue(priority_queue* ready_queue, pid_t pid) {
    // Find in high priority queue
    pcb_node* current = ready_queue->high->head;
    while (current != NULL) {
        if (current->pcb->pid == pid) {
            return current;
        }
        current = current->next;
    }

    // Find in mid priority queue
    current = ready_queue->mid->head;
    while (current != NULL) {
        if (current->pcb->pid == pid) {
            return current;
        }
        current = current->next;
    }

    // Find in low priority queue
    current = ready_queue->low->head;
    while (current != NULL) {
        if (current->pcb->pid == pid) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void deconstruct_queue(pcb_queue* queue) {
    pcb_node* current = queue->head;
    while (current != NULL) {
        pcb_node* tmp = current;
        current = current->next;
        free(tmp);
    }
    free(queue);
}

void deconstruct_priority_queue(priority_queue* ready_queue) {
    deconstruct_queue(ready_queue->high);
    deconstruct_queue(ready_queue->mid);
    deconstruct_queue(ready_queue->low);
    free(ready_queue);
}

pcb_node* get_node_from_ready_queue(priority_queue* ready_queue, pid_t pid) {
    pcb_node* ready_node = get_node_by_pid(ready_queue->high, pid);
    if (ready_node == NULL) {
        ready_node = get_node_by_pid(ready_queue->mid, pid);
        if (ready_node == NULL) {
            ready_node = get_node_by_pid(ready_queue->low, pid);
        } 
    }
    return ready_node;
}

int pick_priority() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); // Get current time
    // Seed random number generator with XOR of sec and nsec fields
    srand(ts.tv_nsec ^ ts.tv_sec); 
    // The ratio of low:mid:high should be 4:6:9. Thus, random generate a int from 1 to 19 and use it to decide which queue to use
    int num = rand() % 18;  // generate a random number between 0 and 18
    if (num <= 3) {
        return LOW;
    } else if (num <= 9) {
        return MID;
    } else {
        return HIGH;
    }
}

void set_stack(stack_t *stack)
{
    void *sp = malloc(SIGSTKSZ);
    VALGRIND_STACK_REGISTER(sp, sp + SIGSTKSZ);

    *stack = (stack_t) { .ss_sp = sp, .ss_size = SIGSTKSZ };
}

int makeContext(ucontext_t *ucp,  void (*func)(), int argc, ucontext_t *next_context, char *argv[]) {
    // intialize the context
    if (getcontext(ucp) == -1) {
        perror("Error in getcontext(context)\n");
        return FAILURE;
    }

    sigemptyset(&ucp->uc_sigmask);
    set_stack(&ucp->uc_stack);
    if (next_context == NULL) {
        ucp->uc_link = NULL;
    } else {
        ucp->uc_link = next_context;
    }

    // set up the stack and instruction pointer for the context
    if (argv == NULL) {
        makecontext(ucp, func, argc);
    } else {
        makecontext(ucp, func, argc, argv);
    }
    return SUCCESS;
}

int printQueue(pcb_queue *queue) {
    pcb_node *node = queue->head;
    printf("######### Start Print Queue ##########\n");
    while (node) {
        printf("process %s with pid = %d\n", node->pcb->pname, node->pcb->pid);
    }
    return 0;
    printf("######### Finish Print Queue ##########\n");
}

int compare_pids(const void* a, const void* b) {
    pcb* pcb_a = *(pcb**)a;
    pcb* pcb_b = *(pcb**)b;
    if (pcb_a->pid < pcb_b->pid) {
        return -1;
    } else if (pcb_a->pid > pcb_b->pid) {
        return 1;
    } else {
        return 0;
    }
}

pcb_queue* sortQueue(pcb_queue* queue) {
    pcb_queue* sorted_queue = (pcb_queue*)malloc(sizeof(pcb_queue));
    sorted_queue->head = NULL;
    sorted_queue->tail = NULL;
    
    // Copy the PCBs from the original queue into an array
    int num_pcbs = 0;
    pcb_node* node;
    for (node = queue->head; node != NULL; node = node->next) {
        num_pcbs++;
    }
    pcb** pcb_array = (pcb**)malloc(num_pcbs * sizeof(pcb*));
    int i = 0;
    for (node = queue->head; node != NULL; node = node->next) {
        pcb_array[i++] = node->pcb;
    }
    
    // Sort the array of PCBs based on pid
    qsort(pcb_array, num_pcbs, sizeof(pcb*), compare_pids);
    
    // Create a new queue and add the sorted PCBs to it
    for (i = 0; i < num_pcbs; i++) {
        pcb_node* new_node = (pcb_node*)malloc(sizeof(pcb_node));
        new_node->pcb = pcb_array[i];
        new_node->next = NULL;
        if (sorted_queue->tail == NULL) {
            sorted_queue->head = new_node;
            sorted_queue->tail = new_node;
        } else {
            sorted_queue->tail->next = new_node;
            sorted_queue->tail = new_node;
        }
    }
    
    // Free the temporary array of PCBs
    free(pcb_array);
    
    return sorted_queue;
}

void add_pcb_to_queue(pcb_queue* queue, pcb* pcb) {
    pcb_node* new_node = (pcb_node*)malloc(sizeof(pcb_node));
    new_node->pcb = pcb;
    new_node->next = NULL;
    if (queue->tail == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
}


pcb_queue* merge_two_queues(pcb_queue* queue1, pcb_queue* queue2) {
    pcb_queue* merged_queue = (pcb_queue*)malloc(sizeof(pcb_queue));
    merged_queue->head = NULL;
    merged_queue->tail = NULL;
    
    // Merge the two sorted queues into a single sorted queue
    pcb_node* node1 = queue1->head;
    pcb_node* node2 = queue2->head;
    while (node1 != NULL && node2 != NULL) {
        if (node1->pcb->pid < node2->pcb->pid) {
            add_pcb_to_queue(merged_queue, node1->pcb);
            node1 = node1->next;
        } else {
            add_pcb_to_queue(merged_queue, node2->pcb);
            node2 = node2->next;
        }
    }
    while (node1 != NULL) {
        add_pcb_to_queue(merged_queue, node1->pcb);
        node1 = node1->next;
    }
    while (node2 != NULL) {
        add_pcb_to_queue(merged_queue, node2->pcb);
        node2 = node2->next;
    }
    
    return merged_queue;
}