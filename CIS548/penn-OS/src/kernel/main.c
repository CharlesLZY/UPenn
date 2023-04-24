#include "scheduler.h"
#include "global.h"
#include "shell.h"

int main(int argc, char const *argv[])
{
    // init system
    if (shell_init(argc, argv) == FAILURE) {
        perror("failed to init shell");
        return FAILURE;
    }

    swapcontext(&main_context, &scheduler_context);

    return 0;
}

