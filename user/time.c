#include "kernel/types.h"
#include "user/user.h"
#include "kernel/pstat.h"


struct rusage { uint cputime; }; // forward (matches kernel/pstat.h)

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: time <command> [args...]\n");
    exit(1);
  }

  int start = uptime();

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "time: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    exec(argv[1], &argv[1]);
    fprintf(2, "time: exec %s failed\n", argv[1]);
    exit(1);
  }

  int status = 0;
  struct rusage ru;
  // wait2 returns child pid on success
  if (wait2(&status, &ru) < 0) {
    fprintf(2, "time: wait2 failed\n");
    exit(1);
  }

  int end = uptime();
  int elapsed = end - start;
  int cpu = (int)ru.cputime;
  int pct = (elapsed > 0) ? (cpu * 100) / elapsed : 0;

  printf("elapsed time: %d ticks , cpu time: %d ticks , %d%% CPU\n",
         elapsed, cpu, pct);
  exit(0);
}
