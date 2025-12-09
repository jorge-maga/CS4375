#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;
  struct proc *p = myproc();

  if(argint(0, &n) < 0)
    return -1;

  addr = p->sz;
  if (n == 0)
    return addr;

  uint64 new_sz = addr + n;
  if(new_sz < p->sz){
    return (uint64)-1;
  }
  p->sz = new_sz;
  /*old eager allocatoin, we don't call growproc right away for lazy allocatoin*/
  /*if(growproc(n) < 0)
    return -1;*/
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_freepmem(void)
{
  uint64 pages = kfreepages_count();
  return pages * PGSIZE;
}

uint64
sys_sem_init(void)
{
  uint64 uaddr;   // user pointer to sem_t
  int pshared;
  int value;
  struct proc *p = myproc();

  // sem_init(sem_t *sem, int pshared, int value)
  if (argaddr(0, &uaddr) < 0 ||
      argint(1, &pshared) < 0 ||
      argint(2, &value) < 0)
    return -1;

  if (value < 0)
    return -1;

  int idx = semalloc();
  if (idx < 0)
    return -1;

  struct semaphore *s = &semtable.sem[idx];
  acquire(&s->lock);
  s->count = value;
  s->valid = 1;
  release(&s->lock);

  // write the semaphore index back to user memory
  sem_t kid = idx;
  if (copyout(p->pagetable, uaddr, (char *)&kid, sizeof(sem_t)) < 0) {
    semdealloc(idx);
    return -1;
  }

  return 0;
}

uint64
sys_sem_wait(void)
{
  uint64 uaddr;   // user pointer to sem_t
  sem_t idx;
  struct proc *p = myproc();

  if (argaddr(0, &uaddr) < 0)
    return -1;

  if (copyin(p->pagetable, (char *)&idx, uaddr, sizeof(sem_t)) < 0)
    return -1;

  if (idx < 0 || idx >= NSEM)
    return -1;

  struct semaphore *s = &semtable.sem[idx];

  acquire(&s->lock);
  if (!s->valid) {
    release(&s->lock);
    return -1;
  }

  // P() operation
  while (s->count == 0) {
    sleep(s, &s->lock);   // atomically sleep & release lock
    if (!s->valid) {
      release(&s->lock);
      return -1;
    }
  }
  s->count--;
  release(&s->lock);

  return 0;
}

uint64
sys_sem_post(void)
{
  uint64 uaddr;   // user pointer to sem_t
  sem_t idx;
  struct proc *p = myproc();

  if (argaddr(0, &uaddr) < 0)
    return -1;

  if (copyin(p->pagetable, (char *)&idx, uaddr, sizeof(sem_t)) < 0)
    return -1;

  if (idx < 0 || idx >= NSEM)
    return -1;

  struct semaphore *s = &semtable.sem[idx];

  acquire(&s->lock);
  if (!s->valid) {
    release(&s->lock);
    return -1;
  }

  s->count++;
  wakeup(s);          // wake any sleepers in sem_wait
  release(&s->lock);

  return 0;
}

uint64
sys_sem_destroy(void)
{
  uint64 uaddr;   // user pointer to sem_t
  sem_t idx;
  struct proc *p = myproc();

  if (argaddr(0, &uaddr) < 0)
    return -1;

  if (copyin(p->pagetable, (char *)&idx, uaddr, sizeof(sem_t)) < 0)
    return -1;

  if (idx < 0 || idx >= NSEM)
    return -1;

  semdealloc(idx);
  return 0;
}
