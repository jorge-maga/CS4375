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
  int n;
  uint64 addr;
  struct proc *p = myproc();

  if(argint(0, &n) < 0)
    return -1;

  addr = p->sz;

  // Basic sanity: don't let sz underflow
  if(n < 0 && (uint64)(-n) > p->sz) {
    return -1;
  }

  // Lazy allocation:
  // only adjust the virtual size (sz), do NOT allocate/free physical pages here.
  p->sz = p->sz + n;

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
sys_mmap(void)
{
  uint64 addr;   // requested addr (we'll ignore it)
  int length;
  int prot, flags, fd, offset;
  struct proc *p = myproc();

  // args: void *addr, uint length, int prot, int flags, int fd, int offset
  if (argaddr(0, &addr) < 0 ||
      argint(1, &length) < 0 ||
      argint(2, &prot) < 0 ||
      argint(3, &flags) < 0 ||
      argint(4, &fd) < 0 ||
      argint(5, &offset) < 0)
    return (uint64)-1;

  if (length <= 0)
    return (uint64)-1;

  // Very simple mmap: just grow the process by 'length' bytes
  // and return the old size as the base address of the new region.
  uint64 oldsz = p->sz;
  if (growproc(length) < 0)
    return (uint64)-1;

  return oldsz;
}

uint64
sys_munmap(void)
{
  uint64 addr;
  int length;

  // args: void *addr, uint length
  if (argaddr(0, &addr) < 0 || argint(1, &length) < 0)
    return -1;

  // Simple stub: do nothing; memory gets freed when the process exits.
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

// return the number of active processes in the system
// fill in user-provided data structure with pid,state,sz,ppid,name
uint64
sys_getprocs(void)
{
  uint64 addr;  // user pointer to struct pstat

  if (argaddr(0, &addr) < 0)
    return -1;
  return(procinfo(addr));
}


uint64 sys_getpriority(void) {
  return myproc()->priority;
}

uint64 sys_setpriority(void) {
  int p;
  argint(0, &p);
  if (p < 0 || p > 49) return -1;
  myproc()->priority = p;
  return 0;
}

uint64
sys_freepmem(void)
{
  return freepmem();
}

uint64
sys_sem_init(void)
{
  uint64 uaddr;
  int pshared;
  int value;
  struct proc *p = myproc();

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
  uint64 uaddr;
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

  while (s->count == 0) {
    sleep(s, &s->lock);
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
  uint64 uaddr;
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
  wakeup(s);
  release(&s->lock);

  return 0;
}

uint64
sys_sem_destroy(void)
{
  uint64 uaddr;
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
