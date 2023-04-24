#include "job-control.h"

/* 
Refer to print_parsed_command() implemented in parser.c provided by the TA.
The only difference is that the '&' will be ignored.
*/
void printCommandLine(struct parsed_command *cmd) {
    for (int i = 0; i < cmd->num_commands; i++) {
        for (char **argument = cmd->commands[i]; *argument != NULL; argument++) {
            printf("%s ", *argument);
        }

        if (i == 0 && cmd->stdin_file != NULL) {
            printf("< %s ", cmd->stdin_file);
        }

        if (i != cmd->num_commands - 1) {
            printf("| ");
        }
    }

    if (cmd->stdout_file != NULL){
        printf(cmd->is_file_append ? ">> %s " : "> %s ", cmd->stdout_file);
    }
    
    puts("");
}

void writeJobStatePrompt(JobState state) {
    char runningPrompt[] = "Running: ";
    char stoppedPrompt[] = "Stopped: ";
    char finishedPrompt[] = "Finished: ";
    char terminatedPrompt[] = "Terminated: ";

    char *statePrompt;
    switch (state) {
        case RUNNING:
            statePrompt = runningPrompt;
            break;
        case STOPPED:
            statePrompt = stoppedPrompt;
            break;
        case FINISHED:
            statePrompt = finishedPrompt;
            break;
        case TERMINATED:
            statePrompt = terminatedPrompt;
            break;
        default:
            printf("state error\n");
            exit(EXIT_FAILURE);
    }

    if (write(STDERR_FILENO, statePrompt, strlen(statePrompt)) == -1) {
        perror("Failed to write the job state prompt");
        exit(EXIT_FAILURE);
    }
}

void writeJobState(Job *job) {
    writeJobStatePrompt(job->state);
    printCommandLine(job->cmd);
}

void pollBackgroundProcesses() {
    /* polling consumes CPU cycles, we only poll once for each shell read */
    int wstatus;
    pid_t pid;
    while ((pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED)) > 0) {
        if (WIFEXITED(wstatus)) { // exit with error
            /* EXIT will never be caught by the poll. */
            Job *job = updateJobList(&_jobList, pid, FINISHED);
            writeJobState(job);
            removeJobList(&_jobList, pid);

        } else if (WIFSTOPPED(wstatus)) { // stopped by a signal
            /* Foreground job process STOP will never be caught by the poll. */
            Job *job = updateJobList(&_jobList, pid, STOPPED);
            writeJobState(job);
            if (runningMode == INTERACTIVE_ASYNC) {
                kill(pid, SIGCONT);
            }

        } else if (WIFSIGNALED(wstatus)) { // terminated by a signal
            /* Foreground job process KILL will never be caught by the poll. */
            #ifdef DEBUG_INFO
            printf("Pid: %d is terminated.\n", pid);
            #endif
            Job *job = updateJobList(&_jobList, pid, TERMINATED);
            writeJobState(job);
            removeJobList(&_jobList, pid);
        }
    } 
}

Job *createJob(struct parsed_command *cmd, pid_t pid, JobState state) {
    Job *newJob = malloc(sizeof(Job));
    newJob->cmd = cmd;
    newJob->pid = pid;
    newJob->state = state;
    return newJob;
}

void initJobList(JobList* jobList) {
    jobList->head = malloc(sizeof(JobListNode));
    jobList->head->job = NULL;
    jobList->head->jobId = 0;

    jobList->tail = malloc(sizeof(JobListNode));
    jobList->tail->job = NULL;
    jobList->tail->jobId = -1;

    jobList->head->prev = NULL;
    jobList->head->next = jobList->tail;
    jobList->tail->prev = jobList->head;
    jobList->tail->next = NULL;

    jobList->jobCount = 0;
}

void appendJobList(JobList *jobList, Job *job) {
    JobListNode *newNode = malloc(sizeof(JobListNode));

    newNode->job = job;
    newNode->jobId = jobList->tail->prev->jobId + 1;

    JobListNode *prev = jobList->tail->prev;
    prev->next = newNode;
    newNode->prev = prev;
    jobList->tail->prev = newNode;
    newNode->next = jobList->tail;
    jobList->jobCount += 1;
}

Job *findJobList(JobList *jobList, pid_t pid) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

Job *updateJobList(JobList *jobList, pid_t pid, JobState state) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {
            curNode->job->state = state;
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

int removeJobList(JobList *jobList, pid_t pid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {

            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;

            free(curNode->job->cmd);
            free(curNode->job);
            free(curNode);

            jobList->jobCount -= 1;
            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}

void clearJobList(JobList *jobList) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        nextNode = curNode->next;

        pid_t pid = curNode->job->pid;
        killpg(pid, SIGKILL);
        int wstatus;
        do {
            waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);
        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

        free(curNode->job->cmd);
        free(curNode->job);
        free(curNode);

        curNode = nextNode;
    }

    free(jobList->head);
    free(jobList->tail);
}


Job *findJobListByJobId(JobList *jobList, int jobId) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->jobId == jobId) {
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

Job *updateJobListByJobId(JobList *jobList, int jobId, JobState state) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->jobId == jobId) {
            curNode->job->state = state;
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

int removeJobListByJobId(JobList *jobList, int jobId) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->jobId == jobId) {

            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;

            free(curNode->job->cmd);
            free(curNode->job);
            free(curNode);

            jobList->jobCount -= 1;
            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}

int removeJobListWithoutFreeCmd(JobList *jobList, pid_t pid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {
            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;


            curNode->job->cmd = NULL;
            free(curNode->job);
            free(curNode);

            jobList->jobCount -= 1;

            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}

Job *popJobList(JobList *jobList, pid_t pid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {
            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;

            Job *job = curNode->job;
            curNode->job = NULL;
            free(curNode);
            jobList->jobCount -= 1;

            return job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

/* 
Refer to print_parsed_command() implemented in parser.c provided by the TA.
*/
void printJobList(JobList *jobList) {
    char runningPrompt[] = "running";
    char stoppedPrompt[] = "stopped";
    char *statePrompt;

    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        int idx = curNode->jobId;
        struct parsed_command *cmd = curNode->job->cmd;
        JobState state = curNode->job->state;

        switch (state) {
            case RUNNING:
                statePrompt = runningPrompt;
                break;
            case STOPPED:
                statePrompt = stoppedPrompt;
                break;
            default:
                printf("ERROR: unexpected background job state");
                exit(EXIT_FAILURE);
        }
        /*
        [idx] commands (state) e.g. [1] sleep 1 (running) 
        */
        printf("[%d] ", idx);

        for (int i = 0; i < cmd->num_commands; ++i) {
            for (char **arguments = cmd->commands[i]; *arguments != NULL; ++arguments)
                printf("%s ", *arguments);

            if (i == 0 && cmd->stdin_file != NULL)
                printf("< %s ", cmd->stdin_file);

            if (i == cmd->num_commands - 1) {
                if (cmd->stdout_file != NULL)
                    printf(cmd->is_file_append ? ">> %s " : "> %s ", cmd->stdout_file);
            } else printf("| ");
        }

        printf(" (%s)", statePrompt);
        puts("");

        curNode = curNode->next;
    }
}

void printAndClearFinishedJobList(JobList *finishedJobList) {
    JobListNode *curNode = finishedJobList->head->next;
    while (curNode != finishedJobList->tail) {
        writeJobState(curNode->job);
        curNode = curNode->next;
    }

    JobListNode *nextNode = finishedJobList->head->next;
    curNode = nextNode;
    while (curNode != finishedJobList->tail) {
        nextNode = curNode->next;
        free(curNode->job->cmd);
        free(curNode->job);
        free(curNode);
        curNode = nextNode;
    }

    finishedJobList->head->next = finishedJobList->tail;
    finishedJobList->tail->prev = finishedJobList->head;
}

CommandType isBuiltinCommand(struct parsed_command *cmd) {
    if (cmd->num_commands == 0){
        return OTHERS;
    }
    if (strcmp(*cmd->commands[0], "bg") == 0) {
        return BG;
    } else if (strcmp(*cmd->commands[0], "fg") == 0) {
        return FG;
    } else if (strcmp(*cmd->commands[0], "jobs") == 0) {
        return JOBS;
    } 
    else if (strcmp(*cmd->commands[0], "kill") == 0) {
        #ifdef ENABLE_KILL
        return KILL;
        #endif
        return OTHERS;
    } 
    else {
        return OTHERS;
    }
}

bool executeBuiltinCommand(struct parsed_command *cmd) {
    CommandType cmdType = isBuiltinCommand(cmd);
    if (cmdType == OTHERS) {
        return false;
    } else {
        switch(cmdType) {
            case BG:
                bgBuildinCommand(cmd);
                break;
            case FG:
                fgBuildinCommand(cmd);
                break;
            case JOBS:
                jobsBuiltinCommand();
                break;
            case KILL:
                #ifdef ENABLE_KILL
                #endif
                break;
            default:
                printf("ERROR: unexpected builtin command");
                return false;
        }
        free(cmd);
        return true;
    }
}

/* 
penn-shell has a notion of the current job. 
If there are stopped jobs, the current job is the most recently stopped one. 
Otherwise, it is the most recently created background job. 
*/
Job *findTheCurrentJob(JobList *jobList) {
    Job *theCurrentJob = NULL;
    JobListNode *curNode = jobList->tail->prev;
    int curNodeId = 0;
    while(curNode != jobList->head) {
        if (curNode->job->state == STOPPED) {
            theCurrentJob = curNode->job;
            break;
        }
        else if (curNode->job->state == RUNNING && curNode->jobId > curNodeId) {
            theCurrentJob = curNode->job;
            curNodeId = curNode->jobId;
        }
        curNode = curNode->prev;
    }
    return theCurrentJob;
}

void bgBuildinCommand(struct parsed_command *cmd) {
    int jobId = (cmd->commands[0][1] != NULL)? atoi(cmd->commands[0][1]) : -1;
    Job *job;
    if (jobId != -1) {
        job = findJobListByJobId(&_jobList, jobId);
    } else { // use the current job by default
        job = findTheCurrentJob(&_jobList);
    }
    if (job == NULL || job->state == RUNNING) {
        printf("ERROR: failed to find the stopped job\n");
    } else {
        killpg(job->pid, SIGCONT);
        updateJobList(&_jobList, job->pid, RUNNING);
        writeJobState(job);
    }
}

void fgBuildinCommand(struct parsed_command *cmd) {
    int jobId = (cmd->commands[0][1] != NULL)? atoi(cmd->commands[0][1]) : -1;
    Job *job;
    if (jobId != -1) {
        job = findJobListByJobId(&_jobList, jobId);
    } else { // use the current job by default
        job = findTheCurrentJob(&_jobList);
    }
    if (job == NULL) {
        printf("ERROR: failed to find the stopped job\n");
    } else {
        fgPid = job->pid;
        if (job->state == STOPPED) {
            printf("Restarting: ");
        }
        printCommandLine(job->cmd);

        pid_t pid = job->pid;
        tcsetpgrp(STDIN_FILENO, pid); // delegate the terminal control
        killpg(pid, SIGCONT);

        int wstatus = 0;
        do {
            waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);
        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus) && !WIFSTOPPED(wstatus));
        
        signal(SIGTTOU, SIG_IGN); // ignore the signal from UNIX when the main process come back from the background to get the terminal control
        tcsetpgrp(STDIN_FILENO, getpid()); // give back the terminal control to the main process
        
        if (WIFEXITED(wstatus)) {
            removeJobList(&_jobList, pid);
        } else if (WIFSIGNALED(wstatus)) {
            writeNewline(); // only need to consider interactive mode
            removeJobList(&_jobList, pid);

        } else if (WIFSTOPPED(wstatus)) {
            
            struct parsed_command *cmd = job->cmd;

            removeJobListWithoutFreeCmd(&_jobList, pid);

            Job *newBackgroundJob = createJob(cmd, pid, STOPPED);
            appendJobList(&_jobList, newBackgroundJob);
            writeNewline();
            writeJobState(newBackgroundJob);

            if (runningMode == INTERACTIVE_ASYNC) {
                kill(pid, SIGCONT);
            }
        }
        fgPid = -1;
    }
}


void jobsBuiltinCommand() {
    printJobList(&_jobList);
}
