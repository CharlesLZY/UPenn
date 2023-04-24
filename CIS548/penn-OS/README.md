# PennOS (23sp-pennOS-group-24)
### Authors

| Name | PennKey |
| --- | --- |
| Ruifan Wang | wang321 |
| Jiaqi Xie | jiaqixie |
| Zhiyuan Liang  | liangzhy |
| Shuo Sun | sunshuo |


### Source Files (Description of Code)
    
    ├── bin/                     # Compiled binaries
    |    |── pennFAT            
    |    └── pennOS
    ├── src/                     # Source code
    |    |── kernel/                
    |    |      |── behavior.c      
    |    |      |── behavior.h      # Shell behaviors
    |    |      |── global.c
    |    |      |── global.h        # global variables
    |    |      |── global2.h       # macros
    |    |      |── job.c
    |    |      |── job.h           # Shell process jobs (e.g. fg, bg)
    |    |      |── kernel.c
    |    |      |── kernel.h        # kernel functions
    |    |      |── log.c
    |    |      |── log.h           # logging functions 
    |    |      |── parser.h
    |    |      |── parser.c
    |    |      |── perrno.h        # self-defined error code and error messages
    |    |      |── perrno.c
    |    |      |── programs.c
    |    |      |── programs.h      # shell built-in commands
    |    |      |── scheduler.c
    |    |      |── scheduler.h     # scheduler for selecting the process to run
    |    |      |── shell.c
    |    |      |── shell.h         # shell initialization
    |    |      |── stress.c
    |    |      |── stress.h     
    |    |      |── user.c
    |    |      |── user.h          # user level functions
    |    |      |── utils.c
    |    |      └── utils.h         # structs and functions related to the structs 
    |    └── PennFAT/
    |           |── FAT.c
    |           |── FAT.h           # file system low level 
    |           |── fd-table.c
    |           |── fd-table.h      # file descriptor table
    |           |── filesys.c
    |           |── filesys.h       # high level file system
    |           |── interface.c
    |           |── interface.h     # user interface for file system
    |           |── pennFAT.c
    |           |── pennFAT.h       # standalone pennFAT
    |           |── utils.c
    |           └── utils.h         # utility functions, global variables
    ├── doc/                     # Documentation files 
    |    └── doc.pdf             # Companion Document
    ├── log/                     # PennOS logs
    |    └── log.txt
    └── README.md

### Extra Credit Answers

- N/A 
  
### Compilation Instructions

* Compile by running (make sure you are in the root directory)
```
make
```
* Run the PennOS
```
./bin/pennOS [filesystem] [logfile]
```
* Or the PennFAT
```
./bin/pennFAT
```

### Additional Logging Events
- DQ_READY_SCHD: removed a process from the ready queue. 
- EQ_READY_TOUT: added a process to the ready queue. 

### Overview of Work Accomplished

PennOS is an UNIX-like operating system created by our team. It has a file system called PennFAT, a priority-based scheduler, and allows users to interact with it through a shell. There are two states in PennOS: kernel land and user land, which can be switched using custom system calls.   
The priority scheduler takes 3-level priorities into account and is implemented using the ucontext library and alarm clock. Users can interact with PennOS using a basic shell that accepts commands and built-ins. The system handles signals from the host operating system and provides a custom signal command called "kill." Additionally, it includes logging functionality for both the scheduler and kernel level functions, and an error handling mechanism.  
A Companion Document in PDF format located in the ./doc directory provides more detailed information about the OS API and functionality. PennOS also supports a FAT file system, allowing basic interactions between the scheduler and the file system.

### Shell Built-in Functions
Built-in user programs
```
cat file ... 
sleep seconds 
busy 
echo string ... 
ls 
touch file ... 
mv src dest 
cp src dest 
rm file ... 
chmod mode file 
ps 
kill -signal_name pid 
zombify 
orphanify 
```
Built-in job control functions
```
nice priority cmd 
nice_pid priority pid 
man 
bg job_id 
fg job_id 
jobs 
logout
```

### General Comments

Very difficult project but we eventually made it through with sweat and blood. One thing bothered us is the floating point exception, we have tried to increase the stack size but it didn't help. It also didn't occur in most of our teammates' computers, just one person who consistently have that issue half of the times. We have tested our code on several laptops. The floating point exception only occurs on the x86 laptop, Macbook with M1/M2 chip can run the code without any bugs. So if you meet the floating point exception, please just re-run it or switch to another non-x86 laptop. We guess the possible reason of this weird behavior is because of the different versions of docker on x86 and amd platforms.

