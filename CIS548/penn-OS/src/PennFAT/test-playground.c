/**
 * @file test-playground.c
 * @author Zhiyuan Liang (liangzhy@seas.upenn.edu)
 * @brief 
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "interface.h"
#include "pennFAT.h"


extern pcb *active_process;

int main(int argc, char **argv) {
    printf("Testing PennFAT ...\n");
    
    fs_mkfs("testFS", 0, 1);
    fs_mount("testFS");


    
    // pf_writeFile("test", 5, "123", PF_APPEND);
    // int dirOffset = fs_touch("test");
    // int res = traceOffset(fs_FATConfig, fs_FAT16InMemory, dirOffset, 513, 255);
    // printf("res: %d\n", res);

    active_process = newPCB();

    int fd = f_open("test", F_WRITE);

    f_write(fd, "12345", 5);


    f_lseek(fd, 5, F_SEEK_END);
    printf("file pointer after lseek end: %d\n", active_process->fds[fd]->fileOffset);

    f_lseek(fd, 5, F_SEEK_SET);
    f_write(fd, "67890", 5);

    f_lseek(fd, 5, F_SEEK_SET);
    printf("file pointer after lseek set: %d\n", active_process->fds[fd]->fileOffset);


    char buf[6] = {'\0'};
    int res = f_read(fd, 2, buf);

    printf("res: %d\n", res);
    printf("buf: %s\n", buf);


    f_close(fd);


    // f_ls("test");

    f_unlink("test");

    f_ls(NULL);

    free(active_process);


    fs_unmount();
    return 0;
}