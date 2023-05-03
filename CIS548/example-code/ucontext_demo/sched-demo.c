/*
    Author: Hannah Pan
    Date:   10/25/2020

    This program demonstrates the use of the ucontext functions in the
    implementation of a round-robin scheduler. (For brevity, error checking is
    omitted.)

    How to compile the program:
        clang -Wall sched-demo.c

    How to run it in Valgrind:
        valgrind a.out [ 2>log ]
*/

#include <signal.h>   // sigaction, sigemptyset, sigfillset, signal
#include <stdio.h>    // dprintf, fputs, perror
#include <stdlib.h>   // malloc, free
#include <sys/time.h> // setitimer
#include <ucontext.h> // getcontext, makecontext, setcontext, swapcontext
#include <unistd.h>   // read, usleep, write

#define THREAD_COUNT 4

static ucontext_t mainContext;
static ucontext_t schedulerContext;
static ucontext_t threadContexts[THREAD_COUNT];
static ucontext_t *activeContext = threadContexts;

static const int centisecond = 10000; // 10 milliseconds

static void scheduler(void)
{
    static int thread = 0;

    thread++;

    if (thread == THREAD_COUNT)
    {
        thread = 0;
        fputs("Thread 0:\n", stderr);
    }

    activeContext = &threadContexts[thread];

    // setting context
    setcontext(activeContext);
    perror("setcontext");
    exit(EXIT_FAILURE);
}

static void cat(void)
{
    fputs("cat: started\n", stderr);

    const size_t size = 4096;
    char buffer[size];

    for (;;)
    {
        const ssize_t n = read(STDIN_FILENO, buffer, size);

        if (n == 0) // Ctrl-D
            break;

        if (n > 0)
            write(STDOUT_FILENO, buffer, n);
    }

    fputs("cat: returning\n", stderr);
}

static void inc(int thread)
{
    for (int i = 0;; i++)
    {
        dprintf(STDERR_FILENO, "%*cThread %d: i = %d\n",
                thread * 20, ' ', thread, i);
        usleep(thread * centisecond);
    }
}

#include <valgrind/valgrind.h>

static void setStack(stack_t *stack)
{
    void *sp = malloc(SIGSTKSZ);
    // Needed to avoid valgrind errors
    VALGRIND_STACK_REGISTER(sp, sp + SIGSTKSZ);

    *stack = (stack_t){.ss_sp = sp, .ss_size = SIGSTKSZ};
}

static void makeContext(ucontext_t *ucp, void (*func)(), int thread)
{
    getcontext(ucp);

    sigemptyset(&ucp->uc_sigmask);
    setStack(&ucp->uc_stack);
    ucp->uc_link = func == cat ? &mainContext : NULL;

    if (thread > 0)
        makecontext(ucp, func, 1, thread);
    else
        makecontext(ucp, func, 0);
}

static void makeContexts(void)
{
    makeContext(&schedulerContext, scheduler, 0);
    makeContext(threadContexts, cat, 0);

    for (int i = 1; i < THREAD_COUNT; i++)
        makeContext(&threadContexts[i], inc, i);
}

static void alarmHandler(int signum) // SIGALRM
{
    swapcontext(activeContext, &schedulerContext);
}

static void setAlarmHandler(void)
{
    struct sigaction act;

    act.sa_handler = alarmHandler;
    act.sa_flags = SA_RESTART;
    sigfillset(&act.sa_mask);

    sigaction(SIGALRM, &act, NULL);
}

static void setTimer(void)
{
    struct itimerval it;

    it.it_interval = (struct timeval){.tv_usec = centisecond * 10};
    it.it_value = it.it_interval;

    setitimer(ITIMER_REAL, &it, NULL);
}

static void freeStacks(void)
{
    free(schedulerContext.uc_stack.ss_sp);

    for (int i = 0; i < THREAD_COUNT; i++)
        free(threadContexts[i].uc_stack.ss_sp);
}

int main(void)
{
    signal(SIGINT, SIG_IGN);  // Ctrl-C
    signal(SIGQUIT, SIG_IGN); /* Ctrl-\ */
    signal(SIGTSTP, SIG_IGN); // Ctrl-Z

    makeContexts();
    setAlarmHandler();
    setTimer();

    swapcontext(&mainContext, activeContext);
    fprintf(stderr, "Back in the main context\n");

    freeStacks();
}
