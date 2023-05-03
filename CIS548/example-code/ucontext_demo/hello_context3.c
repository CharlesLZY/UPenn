/*
* How to compile the program: 
*     gcc hello_context3.c
*
* Run with:
*     ./a.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <signal.h>
#include <valgrind/valgrind.h>

#define STACKSIZE 1000

ucontext_t *storedContext, *nextToRun, *mainContext;
// function which u0 runs
static void f()
{
  printf("Hello World\n");
  swapcontext(storedContext, nextToRun);
  printf("Back in first context (u0)\n");
}
// function which u1 runs
static void f1(int in)
{
  printf("Goodbye World\n");
  printf("The input is %d \n", in);
  swapcontext(nextToRun, storedContext);
}

int main(int argc, char *argv[])
{
  storedContext = malloc(sizeof(ucontext_t));
  mainContext = malloc(sizeof(ucontext_t));

  ucontext_t *uc0 = malloc(sizeof(ucontext_t));
  ucontext_t *uc1 = malloc(sizeof(ucontext_t));
  nextToRun = uc1;

  void *stack0;
  void *stack1;

  getcontext(uc0);
  getcontext(uc1);

  stack0 = malloc(STACKSIZE);
  stack1 = malloc(STACKSIZE);

  VALGRIND_STACK_REGISTER(stack0, stack0 + STACKSIZE);
  VALGRIND_STACK_REGISTER(stack1, stack1 + STACKSIZE);

  uc0->uc_stack.ss_sp = stack0;
  uc0->uc_stack.ss_size = STACKSIZE;
  uc0->uc_stack.ss_flags = 0;

  uc1->uc_stack.ss_sp = stack1;
  uc1->uc_stack.ss_size = STACKSIZE;
  uc1->uc_stack.ss_flags = 0;

  sigemptyset(&(uc0->uc_sigmask));
  sigemptyset(&(uc1->uc_sigmask));

  uc0->uc_link = mainContext;
  uc1->uc_link = NULL;

  // char toPass[10] = "in func";
  int i = 10;

  // casting f1 as f1_void to pass it into makecontext
  void *f1_void = (void (*)(void))f1;
  makecontext(uc0, f, 0);
  makecontext(uc1, f1_void, 2, i);

  swapcontext(mainContext, uc0);

  printf("Back in original context\n");

  free(uc0->uc_stack.ss_sp);
  free(uc1->uc_stack.ss_sp);
  free(uc1);
  free(uc0);
  free(mainContext);
  free(storedContext);
  return 0;
}
