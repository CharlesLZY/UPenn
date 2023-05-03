/*
* How to compile the program: 
*     gcc hello_context.c
*
* Run with:
*     ./a.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <signal.h>


#define STACKSIZE 1000

void f(){
  printf("Hello World\n");
}

int main(int argc, char * argv[]){
  ucontext_t uc;
  void * stack;

  getcontext(&uc);

  stack = malloc(STACKSIZE);

  uc.uc_stack.ss_sp = stack;
  uc.uc_stack.ss_size = STACKSIZE;
  uc.uc_stack.ss_flags = 0;

  sigemptyset(&(uc.uc_sigmask));

  uc.uc_link = NULL;

  makecontext(&uc, f, 0);

  setcontext(&uc);
  perror("setcontext");

  return 0;
}
