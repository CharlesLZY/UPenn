#include "perrno.h"

int p_errno;  // error number

void p_set_errno(int errnum) {
    p_errno = errnum;
}

void p_perror(const char* error_msg) {
    switch (p_errno)
    {
        case 0:
            printf("[Success]: %s\n", error_msg);
            break;
        case -1:
            printf("[No Such Process]: %s\n", error_msg);
            break;
        case -2:
            printf("[No Such File]: %s\n", error_msg);
            break;
        case -3:
            printf("[Failed to initialize queue]: %s\n", error_msg);
            break;
        case -4:
            printf("[Should not kill shell]: %s\n", error_msg);
            break;
        case -5:
            printf("[Node is null]: %s\n", error_msg);
            break;
        case -6:
            printf("[Parent is null]: %s\n", error_msg);
            break;
        case -7:
            printf("[Process is null]: %s\n", error_msg);
            break;
        case -8:
            printf("[Process is not in ready queue]: %s\n", error_msg);
            break;
        case -9:
            printf("[Process is not in stopped queue]: %s\n", error_msg);
            break;
        case -10:
            printf("[Failed to cleanup]: %s\n", error_msg);
            break;
        default:
            break;
    }
}

void p_reset_errno() {
    p_errno = 0;
}