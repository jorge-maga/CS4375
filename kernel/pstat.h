#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "types.h"   // for uint

struct rusage {
  uint cputime;   // ticks the proc actually ran on CPU (user mode)
};

#endif
