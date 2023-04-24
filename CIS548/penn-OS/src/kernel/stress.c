/******************************************************************************
 *                                                                            *
 *                             Author: Hannah Pan                             *
 *                             Date:   04/15/2021                             *
 *                                                                            *
 ******************************************************************************/


#include "stress.h"

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>


/******************************************************************************
 *                                                                            *
 *  Replace kernel.h with your own header file(s) for p_spawn and p_waitpid.  *
 *                                                                            *
 ******************************************************************************/

#include "kernel.h"
#include "user.h"

static void nap(void)
{
  log_event(active_process, "NAP");
  usleep(10000);  // 10 milliseconds
}


/*
 * The function below spawns 10 nappers named child_0 through child_9 and waits
 * on them. The wait is non-blocking if nohang is true, or blocking otherwise.
 */

static void spawn(bool nohang)
{
  char name[] = "child_0";
  char *argv[] = { name, NULL };
  int pid = 0;

  // Spawn 10 nappers named child_0 through child_9.
  for (int i = 0; i < 10; i++) {
    argv[0][sizeof name - 2] = '0' + i;
    const int id = p_spawn(nap, argv, F_STDIN_FD, F_STDOUT_FD);

    if (i == 0){
      pid = id;
    }

    dprintf(STDERR_FILENO, "%s was spawned\n", *argv);
  }

  // Wait on all children.
  while (1) {
    const int cpid = p_waitpid(-1, NULL, nohang);

    if (cpid < 0)  // no more waitable children (if block-waiting) or error
      break;

    // polling if nonblocking wait and no waitable children yet
    if (nohang && cpid == 0) {
      usleep(90000);  // 90 milliseconds
      continue;
    }

    dprintf(STDERR_FILENO, "child_%d was reaped\n", cpid - pid);
  }
}


/*
 * The function below recursively spawns itself 26 times and names the spawned
 * processes Gen_A through Gen_Z. Each process is block-waited by its parent.
 */

static void spawn_r(void)
{
  static int i = 0;

  int pid = 0;
  char name[] = "Gen_A";
  char *argv[] = { name, NULL };

  if (i < 26) {
    argv[0][sizeof name - 2] = 'A' + i++;
    pid = p_spawn(spawn_r, argv, F_STDIN_FD, F_STDOUT_FD);
    dprintf(STDERR_FILENO, "%s was spawned\n", *argv);
    usleep(10000);  // 10 milliseconds
  }

  if (pid > 0 && pid == p_waitpid(pid, NULL, false))
    dprintf(STDERR_FILENO, "%s was reaped\n", *argv);
}


/******************************************************************************
 *                                                                            *
 * Add commands hang, nohang, and recur to the shell as built-in subroutines  *
 * which call the following functions, respectively.                          *
 *                                                                            *
 ******************************************************************************/

void hang(void)
{
  spawn(false);
}

void nohang(void)
{
  spawn(true);
}

void recur(void)
{
  spawn_r();
}
