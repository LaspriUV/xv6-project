#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"

int sys_getpstats(void)
{
  struct pstat *ps;

  // argptr: obtener el primer argumento de la syscall,
  // que debe ser un puntero válido en espacio de usuario.
  if (argptr(0, (void *)&ps, sizeof(*ps)) < 0)
    return -1;

  acquire(&ptable.lock);

  struct proc *p;
  int i = 0;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++, i++)
  {
    ps->inuse[i] = (p->state != UNUSED);
    ps->pid[i] = p->pid;
    ps->arrival_time[i] = p->arrival_time;
    ps->finish_time[i] = p->finish_time;
    ps->run_ticks[i] = p->run_ticks;
    ps->wait_ticks_tot[i] = p->wait_ticks_tot;
    ps->ctx_switches[i] = p->ctx_switches;
  }

  release(&ptable.lock);
  return 0;
}

int sys_fork(void)
{
  return fork();
}

int sys_exit(void)
{
  exit();
  return 0; // not reached
}

int sys_wait(void)
{
  return wait();
}

int sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void)
{
  return myproc()->pid;
}

int sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
