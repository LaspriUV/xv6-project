#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

// NOTE: ptable is defined in proc.c as:
// struct { struct spinlock lock; struct proc proc[NPROC]; } ptable;
// We need to refer to ptable.lock and ptable.proc here.
extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

void
tvinit(void)
{
  int i;
  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  // Flag to indicate whether we should yield after releasing locks.
  int need_yield = 0;

  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    // tick counter for the system (only CPU 0 increments the global ticks)
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();

    // --- Begin scheduling-related per-tick updates ---
    {
      struct proc *p;

      // Acquire process table lock to safely update per-process counters and priorities.
      acquire(&ptable.lock);

      // Aging: increment wait_ticks for RUNNABLE processes and apply aging when threshold reached.
      for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        if(p->state == RUNNABLE){
          p->wait_ticks++;
          if(p->wait_ticks >= AGING_THRESHOLD){
            if(p->priority > MIN_PRIORITY){
              p->priority--;      // improve priority (0 is best)
            }
            p->wait_ticks = 0;
          }
        }
      }

      // Quantum & penalization: update cpu_ticks of the currently running process.
      // Use mycpu()->proc or myproc(); ensure the process is RUNNING.
      if(myproc() && myproc()->state == RUNNING){
        myproc()->cpu_ticks++;
        if(myproc()->cpu_ticks >= QUANTUM_TICKS){
          // Penalize: make priority worse (increase number), capped at MAX_PRIORITY
          if(myproc()->priority < MAX_PRIORITY)
            myproc()->priority++;
          myproc()->cpu_ticks = 0;
          // Mark that we should yield (preempt) after releasing the lock.
          need_yield = 1;
        }
      }

      release(&ptable.lock);
    }
    // --- End scheduling updates ---

    break;

  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  
  //PAGEBREAK: 13
  default:
    // Handle page fault (T_PGFLT = 14)
    if(tf->trapno == T_PGFLT){
      uint va = rcr2();  // Get the faulting address from CR2
      
      // Check if this is a write fault (error code bit 1)
      // Error code bits: bit 0 = present, bit 1 = write, bit 2 = user
      if(myproc() && (tf->err & 2)){  // Write fault
        // Try to handle as COW fault
        if(cowhandler(va) == 0){
          // Successfully handled COW fault, continue execution
          break;
        }
      }
      // If not a COW fault or handler failed, fall through to kill process
    }
    
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // === NOTE ===
  // We removed the old unconditional yield() on every timer tick here,
  // because now preemption is decided in the timer handling above
  // and we only call yield() when need_yield==1 (after releasing ptable.lock).
  if(need_yield){
    yield();
  }

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER){
    cprintf("Process %d (%s) being killed\n", myproc()->pid, myproc()->name);
    exit();
  }
}