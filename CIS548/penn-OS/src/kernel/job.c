/**
 * @file job.c
 * @author Shuo Sun (sunshuo@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "job.h"

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

/* In this project, Terminated should never occur */
void writeJobStatePrompt(JobState state) {
    char runningPrompt[] = "Running: ";
    char stoppedPrompt[] = "Stopped: ";
    char finishedPrompt[] = "Finished: ";
    char terminatedPrompt[] = "Terminated: ";

    char *statePrompt;
    switch (state) {
        case JOB_RUNNING:
            statePrompt = runningPrompt;
            break;
        case JOB_STOPPED:
            statePrompt = stoppedPrompt;
            break;
        case JOB_FINISHED:
            statePrompt = finishedPrompt;
            break;
        case JOB_TERMINATED:
            statePrompt = terminatedPrompt;
            break;
        default:
            printf("state error\n");
            exit(EXIT_FAILURE);
    }

    f_write(F_ERROR, statePrompt, strlen(statePrompt));
}


void writeJobState(Job *job) {
    writeJobStatePrompt(job->state);
    printCommandLine(job->cmd);
}

void writeNewline() {
    f_write(F_ERROR, "\n", 1);
}

/* Refer to print_parsed_command() implemented in parser.c provided by the TA */
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
            case JOB_RUNNING:
                statePrompt = runningPrompt;
                break;
            case JOB_STOPPED:
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

CommandType parseBuiltinCommandType(struct parsed_command *cmd) {
    if (strcmp(*cmd->commands[0], "bg") == 0) {
        return BG;
    } 
    else if (strcmp(*cmd->commands[0], "fg") == 0) {
        return FG;
    } 
    else if (strcmp(*cmd->commands[0], "jobs") == 0) {
        return JOBS;
    } 
    else if (strcmp(*cmd->commands[0], "nice") == 0) {
        return NICE;
    }
    else if (strcmp(*cmd->commands[0], "nice_pid") == 0) {
        return NICE_PID;
    }
    else if (strcmp(*cmd->commands[0], "man") == 0) {
        return MAN;
    }
    else if (strcmp(*cmd->commands[0], "logout") == 0) {
        return LOGOUT;
    }
    else if (strcmp(*cmd->commands[0], "kill") == 0) {
        return KILL;
    }
    else {
        return OTHERS;
    }
}

void killBuildinCommand(struct parsed_command *cmd) {
    s_kill(cmd->commands[0]);
}

/* Execute built-in command wrapped up */
CommandType executeBuiltinCommand(struct parsed_command *cmd) {
    CommandType cmdType = parseBuiltinCommandType(cmd);

    switch(cmdType) {
        case NICE:
            /* move the logic to outside */
            return NICE;
        case NICE_PID:
            nicePidBuildinCommand(cmd);
            return NICE_PID;
        case MAN:
            manBuildinCommand();
            return MAN;
        case BG:
            bgBuildinCommand(cmd);
            return BG;
        case FG:
            fgBuildinCommand(cmd);
            return FG;
        case JOBS:
            jobsBuildinCommand();
            return JOBS;
        case KILL:
            killBuildinCommand(cmd);
            return KILL;
        case LOGOUT:
            /* move the logic to outside */
            return LOGOUT;
        default:
            printf("ERROR: OTHER COMMAND\n");
            free(cmd);
            return OTHERS;
    }
    return OTHERS;
}


void jobsBuildinCommand() {
    printJobList(&_jobList);
}

void bgBuildinCommand(struct parsed_command *cmd) {
    int jobId = (cmd->commands[0][1] != NULL)? atoi(cmd->commands[0][1]) : -1;
    Job *job;
    if (jobId != -1) {
        job = findJobListByJobId(&_jobList, jobId);
    } else { // use the current job by default
        job = findTheCurrentJob(&_jobList);
    }
    if (job == NULL || job->state == JOB_RUNNING) {
        printf("ERROR: failed to find the stopped job\n");
    } else {
        p_kill(job->pid, S_SIGCONT);
        updateJobList(&_jobList, job->pid, JOB_RUNNING);
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
        if (job->state == JOB_STOPPED) {
            printf("Restarting: ");
        }
        printCommandLine(job->cmd);
        
        pid_t pid = job->pid;
        p_kill(pid, S_SIGCONT);

        int wstatus = 0;
        p_waitpid(pid, &wstatus, false);
        
        if (W_WIFEXITED(wstatus)) {
            removeJobList(&_jobList, pid);
        } else if (W_WIFSIGNALED(wstatus)) {
            writeNewline(); // only need to consider interactive mode
            removeJobList(&_jobList, pid);
        } else if (W_WIFSTOPPED(wstatus)) {
            struct parsed_command *cmd = job->cmd;
            removeJobListWithoutFreeCmd(&_jobList, pid);
            Job *newBackgroundJob = createJob(cmd, pid, JOB_STOPPED);
            appendJobList(&_jobList, newBackgroundJob);
            writeNewline();
            writeJobState(newBackgroundJob);
        }
        fgPid = -1;
    }
}

// void niceBuildinCommand(struct parsed_command *cmd) {
//     int priority = atoi(cmd->commands[0][1]);
    
// }

void nicePidBuildinCommand(struct parsed_command *cmd) {
    int priority = atoi(cmd->commands[0][1]);
    pid_t pid = atoi(cmd->commands[0][2]);
    p_nice(pid, priority);
}

void manBuildinCommand() {
    printf("cat file ... \n");
    printf("sleep seconds \n");
    printf("busy \n");
    printf("echo string ... \n");
    printf("ls \n");
    printf("touch file ... \n");
    printf("mv src dest \n");
    printf("cp src dest \n");
    printf("rm file ... \n");
    printf("chmod mode file \n");
    printf("ps \n");
    printf("kill -signal_name pid \n");
    printf("zombify \n");
    printf("orphanify \n");
    printf("nice priority cmd \n");
    printf("nice_pid priority pid \n");
    printf("man \n");
    printf("bg job_id \n");
    printf("fg job_id \n");
    printf("jobs \n");
    printf("logout \n");
}

// void logoutBuiltinCommand() {

// }



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

void clearJobList(JobList *jobList) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        nextNode = curNode->next;

        pid_t pid = curNode->job->pid;
        p_kill(pid, S_SIGTERM);
        int wstatus;

        p_waitpid(pid, &wstatus, false);
        free(curNode->job->cmd);
        free(curNode->job);
        free(curNode);

        curNode = nextNode;
    }

    free(jobList->head);
    free(jobList->tail);
}

void pollBackgroundProcesses() {
    int wstatus;
    pid_t pid;
    pid = p_waitpid(-1, &wstatus, true);
    while (pid > 0) {
        if (W_WIFEXITED(wstatus)) {
            Job *job = updateJobList(&_jobList, pid, JOB_FINISHED);
            writeJobState(job);
            removeJobList(&_jobList, pid);
        } else if (W_WIFSIGNALED(wstatus)) {
            Job *job = updateJobList(&_jobList, pid, JOB_TERMINATED);
            writeJobState(job);
            removeJobList(&_jobList, pid);
        } else if (W_WIFSTOPPED(wstatus)) {
            Job *job = updateJobList(&_jobList, pid, JOB_STOPPED);
            writeJobState(job);
        }
        pid = p_waitpid(-1, &wstatus, true);
    }
    
}

Job *findTheCurrentJob(JobList *jobList) {
    Job *curJob = NULL;
    JobListNode *curNode = jobList->tail->prev;
    int curNodeId = 0;

    while (curNode != jobList->head) {
        if (curNode->job->state == JOB_STOPPED) {
            curJob = curNode->job;
            break;
        } else if (curNode->job->state == JOB_RUNNING && curNode->jobId > curNodeId) {
            curJob = curNode->job;
            curNodeId = curNode->jobId;
        }
        curNode = curNode->prev;
    }

    return curJob;
}

