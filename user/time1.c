#include "kernel/types.h"
#include "user/user.h"
#include "kernel/pstat.h"


int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: time1 <command> [args...]\n");
    exit(1);
  }

  int start = uptime();  // ticks before fork

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "time1: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    // child: exec the target command; argv[1]..argv[argc-1]
    exec(argv[1], &argv[1]);
    fprintf(2, "time1: exec %s failed\n", argv[1]);
    exit(1);
  }

  // parent
  int status = 0;
  wait(&status);
  int end = uptime();    // ticks after child finished

  printf("elapsed time: %d ticks\n", end - start);
  exit(0);
}
