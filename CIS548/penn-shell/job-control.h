#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include "utils.h"

typedef enum {
    NON_INTERACTIVE,
    INTERACTIVE_SYNC,
    INTERACTIVE_ASYNC // mandatory additional credit
} RunningMode;

typedef enum {
    RUNNING,
    STOPPED,
    FINISHED,
    TERMINATED
} JobState;

typedef enum {
    BG, // Syntax: bg [job_id]
    FG, // Syntax: fg [job_id]
    JOBS, // Syntax: jobs
    KILL, // Syntax: kill [job_id]
    OTHERS // non-builtin command
} CommandType;

/*
shell               -->            P
                                   |
                                   |
job                 -->           P_0
                                /  |  \
                               /   |   \
cmd1 | cmd2 | ...   -->      P_1  P_2 ...

By default, child process will inherbit its parent process's pgid
For the job abstraction, P_0's pgid will be set to its pid and 
P_0's child processes will inherbit its pgid.
*/

typedef struct Job {
    struct parsed_command *cmd;
    pid_t pid; // job process's pgid is set to be its pid
    JobState state;
} Job;

/*
We use linked list as the working around data structure to store jobs.
*/
typedef struct JobListNode {
    Job *job;
    struct JobListNode *prev;
    struct JobListNode *next;

    /* 
    jobId is unique among all background jobs. 
    jobId remains constant for the duration of the job. 
    The job id for a new background job is always 
    that of the most recently created background job plus 1.
    */
    int jobId;
} JobListNode;

typedef struct JobList {
    JobListNode *head;
    JobListNode *tail;
    int jobCount;
} JobList;

extern RunningMode runningMode;
extern JobList _jobList; // store all background job
extern JobList _finishedJobList; // store all finished background job for async mode
extern pid_t fgPid; // running foreground process

void printCommandLine(struct parsed_command *cmd);
void writeJobStatePrompt(JobState state);
void writeJobState(Job *job);

void pollBackgroundProcesses();

Job *createJob(struct parsed_command *cmd, pid_t pid, JobState state);
void initJobList(JobList *jobList);
void appendJobList(JobList *jobList, Job *job);
Job *findJobList(JobList *jobList, pid_t pid);
Job *updateJobList(JobList *jobList, pid_t pid, JobState state);
int removeJobList(JobList *jobList, pid_t pid);
void clearJobList(JobList *jobList);

Job *findJobListByJobId(JobList *jobList, int jobId);
Job *updateJobListByJobId(JobList *jobList, int jobId, JobState state);
int removeJobListByJobId(JobList *jobList, int jobId);
int removeJobListWithoutFreeCmd(JobList *jobList, pid_t pid);
Job *popJobList(JobList *jobList, pid_t pid);

void printJobList(JobList *jobList);
void printAndClearFinishedJobList(JobList *finishedJobList);

CommandType isBuiltinCommand(struct parsed_command *cmd);

bool executeBuiltinCommand(struct parsed_command *cmd);

/* 
penn-shell has a notion of the current job. 
If there are stopped jobs, the current job is the most recently stopped one. 
Otherwise, it is the most recently created background job. 
*/
Job *findTheCurrentJob(JobList *jobList);
void bgBuildinCommand(struct parsed_command *cmd);
void fgBuildinCommand(struct parsed_command *cmd);
void jobsBuiltinCommand();
#endif