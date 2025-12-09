#include "param.h"

// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

struct semaphore {
  struct spinlock lock;
  int count;
  int valid;   // 1 if allocated, 0 if free
};

struct semtab {
  struct spinlock lock;
  struct semaphore sem[NSEM];
};

extern struct semtab semtable;
