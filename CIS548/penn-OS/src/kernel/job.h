/**
 * @file job.h
 * @author Shuo Sun (sunshuo@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef JOB_H
#define JOB_H

#include "user.h"
#include "utils.h"
#include "parser.h"
#include "unistd.h"
#include "../PennFAT/interface.h"
#include "programs.h"

extern JobList _jobList; // store all background job
extern pid_t fgPid; // running foreground process

/* Utility functions for writing job state */
/**
 * @brief The state of the job
 * 
 * @param cmd 
 */
void printCommandLine(struct parsed_command *cmd);
/**
 * @brief Write the job state and prompt to the screen
 * 
 * @param state 
 */
void writeJobStatePrompt(JobState state);
/**
 * @brief Write the job state to the screen
 * 
 * @param job 
 */
void writeJobState(Job *job);
/**
 * @brief Write the a new line to the screen
 * 
 */
void writeNewline();

/* Utility functions for job and job list */
/**
 * @brief Create a Job object
 * 
 * @param cmd 
 * @param pid 
 * @param state 
 * @return Job* 
 */
Job *createJob(struct parsed_command *cmd, pid_t pid, JobState state);
/**
 * @brief Initialize the job list
 * 
 * @param jobList 
 */
void initJobList(JobList *jobList);
/**
 * @brief Append a job to the job list
 * 
 * @param jobList 
 * @param job 
 */
void appendJobList(JobList *jobList, Job *job);
/**
 * @brief Find a job in the job list
 * 
 * @param jobList 
 * @param pid 
 * @return Job* 
 */
Job *findJobList(JobList *jobList, pid_t pid);
/**
 * @brief Update the state of a job in the job list
 * 
 * @param jobList 
 * @param pid 
 * @param state 
 * @return Job* 
 */
Job *updateJobList(JobList *jobList, pid_t pid, JobState state);
/**
 * @brief Remove a job from the job list
 * 
 * @param jobList 
 * @param pid 
 * @return int 
 */
int removeJobList(JobList *jobList, pid_t pid);

/**
 * @brief Find a job in the job list by job id
 * 
 * @param jobList 
 * @param jobId 
 * @return Job* 
 */
Job *findJobListByJobId(JobList *jobList, int jobId);
/**
 * @brief Update the state of a job in the job list by job id
 * 
 * @param jobList 
 * @param jobId 
 * @param state 
 * @return Job* 
 */
Job *updateJobListByJobId(JobList *jobList, int jobId, JobState state);
/**
 * @brief Remove a job from the job list by job id
 * 
 * @param jobList 
 * @param jobId 
 * @return int 
 */
int removeJobListByJobId(JobList *jobList, int jobId);
/**
 * @brief Remove a job from the job list without free the cmd
 * 
 * @param jobList 
 * @param pid 
 * @return int 
 */
int removeJobListWithoutFreeCmd(JobList *jobList, pid_t pid);
/**
 * @brief Pop a job from the job list
 * 
 * @param jobList 
 * @param pid 
 * @return Job* 
 */
Job *popJobList(JobList *jobList, pid_t pid);
/**
 * @brief Find the current job
 * 
 * @param jobList 
 * @return Job* 
 */
Job *findTheCurrentJob(JobList *jobList);

/**
 * @brief Print the job list
 * 
 * @param jobList 
 */
void printJobList(JobList *jobList);

/* Built-in commands */
/**
 * @brief Parse the built-in command type
 * 
 * @param cmd 
 * @return CommandType 
 */
CommandType parseBuiltinCommandType(struct parsed_command *cmd);
/**
 * @brief Execute the built-in command
 * 
 * @param cmd 
 * @return CommandType 
 */
CommandType executeBuiltinCommand(struct parsed_command *cmd);


/**
 * @brief Clear the job list
 * 
 * @param jobList 
 */
void clearJobList(JobList *jobList);
/**
 * @brief Poll the background processes
 * 
 */
void pollBackgroundProcesses();

/**
 * @brief Built-in command: bg
 * 
 * @param cmd 
 */
void bgBuildinCommand(struct parsed_command *cmd);
/**
 * @brief Built-in command: fg
 * 
 * @param cmd 
 */
void fgBuildinCommand(struct parsed_command *cmd);
/**
 * @brief Built-in command: jobs
 * 
 */
void jobsBuildinCommand();
/**
 * @brief Built-in command: nice
 * 
 * @param cmd 
 */
void nicePidBuildinCommand(struct parsed_command *cmd);
/**
 * @brief Built-in command: kill
 * 
 * @param cmd 
 */
void killBuildinCommand(struct parsed_command *cmd);
/**
 * @brief Built-in command: man
 * 
 */
void manBuildinCommand();


#endif