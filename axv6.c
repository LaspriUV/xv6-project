#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define NPROCS 10 // cuantos procesos de prueba crear

void cpu_bound_job(void)
{
    volatile int i, j;
    for (i = 0; i < 100000; i++)
        j = i * i;
}

int main(int argc, char *argv[])
{
    int i, pid;
    struct pstat ps;

    // 1) Crear NPROCS procesos CPU-bound
    for (i = 0; i < NPROCS; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            printf(1, "fork failed\n");
            exit();
        }
        if (pid == 0)
        {
            cpu_bound_job();
            exit();
        }
    }

    // 2) Esperar a que todos terminen
    for (i = 0; i < NPROCS; i++)
        wait();

    // 3) Obtener snapshot de métricas
    if (getpstats(&ps) < 0)
    {
        printf(1, "getpstats failed\n");
        exit();
    }

    uint total_TAT = 0;
    uint total_WT = 0;
    int count = 0;

    printf(1, "PID\tTAT\tWT\tCTX\n");

    // 4) Recorrer todas las entradas de ptable
    for (i = 0; i < NPROC; i++)
    {
        if (!ps.inuse[i])
            continue;
        if (ps.finish_time[i] == 0) // procesos que aún no terminaron
            continue;

        uint tat = ps.finish_time[i] - ps.arrival_time[i];
        uint wt = ps.wait_ticks_tot[i];

        printf(1, "%d\t%d\t%d\t%d\n",
               ps.pid[i], tat, wt, ps.ctx_switches[i]);

        total_TAT += tat;
        total_WT += wt;
        count++;
    }

    if (count > 0)
    {
        printf(1, "AVG TAT = %d\n", total_TAT / count);
        printf(1, "AVG WT  = %d\n", total_WT / count);
    }

    exit();
}
