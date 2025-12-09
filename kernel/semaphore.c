#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"

struct semtab semtable;

// Initialize semaphore table
void
seminit(void)
{
  initlock(&semtable.lock, "semtable");
  for (int i = 0; i < NSEM; i++) {
    initlock(&semtable.sem[i].lock, "sem");
    semtable.sem[i].count = 0;
    semtable.sem[i].valid = 0;
  }
}

// Allocate a semaphore; return index or -1 if none
int
semalloc(void)
{
  acquire(&semtable.lock);
  for (int i = 0; i < NSEM; i++) {
    if (semtable.sem[i].valid == 0) {
      semtable.sem[i].valid = 1;
      semtable.sem[i].count = 0;
      release(&semtable.lock);
      return i;
    }
  }
  release(&semtable.lock);
  return -1;
}

// Deallocate semaphore at index idx
void
semdealloc(int idx)
{
  if (idx < 0 || idx >= NSEM)
    return;

  acquire(&semtable.lock);
  semtable.sem[idx].valid = 0;
  semtable.sem[idx].count = 0;
  release(&semtable.lock);
}
