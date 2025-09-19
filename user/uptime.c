// user/uptime.c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int t = uptime();   // system call: returns clock ticks since boot
  printf("up %d clock ticks\n", t);
  exit(0);
}
