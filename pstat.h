#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat
{
    int inuse[NPROC];           // 1 si ptable.proc[i] está en uso (no UNUSED)
    int pid[NPROC];             // PID del proceso
    uint arrival_time[NPROC];   // primer tick RUNNABLE
    uint finish_time[NPROC];    // tick cuando terminó
    uint run_ticks[NPROC];      // ticks en RUNNING
    uint wait_ticks_tot[NPROC]; // ticks en RUNNABLE
    uint ctx_switches[NPROC];   // cambios de contexto sufridos
};

#endif
