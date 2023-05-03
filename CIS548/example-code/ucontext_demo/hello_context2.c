/*
* Simple demo with 2 threads
* 
* How to compile the program: 
*     gcc hello_context2.c
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
void f1(){
  printf("Goodbye World\n");
}

int main(int argc, char * argv[]){
  ucontext_t uc0, uc1;
  void * stack0;
  void * stack1;

  getcontext(&uc0);
  getcontext(&uc1);

  stack0 = malloc(STACKSIZE);
  stack1 = malloc(STACKSIZE);

  uc0.uc_stack.ss_sp = stack0;
  uc0.uc_stack.ss_size = STACKSIZE;
  uc0.uc_stack.ss_flags = 0;

  uc1.uc_stack.ss_sp = stack1;
  uc1.uc_stack.ss_size = STACKSIZE;
  uc1.uc_stack.ss_flags = 0;


  sigemptyset(&(uc0.uc_sigmask));
  sigemptyset(&(uc1.uc_sigmask));

  uc0.uc_link = &uc1;
  uc1.uc_link = NULL;

  makecontext(&uc0, f, 0);
  makecontext(&uc1, f1, 0);

  setcontext(&uc0);
  perror("setcontext");

  return 0;
}
