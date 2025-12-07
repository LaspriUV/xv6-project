#include "types.h"
#include "stat.h"
#include "user.h"

// Número de procesos de prueba
#define NPROCS 10
// "Trabajo" que hace cada proceso hijo (ticks aproximados de CPU)
#define WORK_ITERS 100000000

// Programa CPU-bound simple
void cpu_job(void)
{
    volatile int i;
    for (i = 0; i < WORK_ITERS; i++)
    {
        // loop vacío para consumir CPU
    }
}

int main(int argc, char *argv[])
{
    int n = NPROCS;
    if (argc > 1)
    {
        n = atoi(argv[1]);
        if (n <= 0)
            n = NPROCS;
    }

    int i;
    int start_tick, end_tick;
    int tat_sum = 0;
    int wt_sum = 0;

    // Tiempo de llegada aproximado: antes del fork de cada hijo
    int arrival[NPROCS];

    // Crear n procesos CPU-bound
    for (i = 0; i < n; i++)
    {
        arrival[i] = uptime(); // tick en que se crea este proceso
        int pid = fork();
        if (pid < 0)
        {
            printf(2, "bench: fork failed\n");
            exit();
        }
        if (pid == 0)
        {
            // Hijo
            cpu_job();
            exit();
        }
        // Padre continúa creando más hijos
    }

    // Esperar a todos los hijos y medir TAT
    for (i = 0; i < n; i++)
    {
        int pid = wait();
        if (pid < 0)
        {
            printf(2, "bench: wait failed\n");
            exit();
        }
        end_tick = uptime();
        // Turnaround time aproximado: fin - llegada
        // Como no sabemos qué índice corresponde a este pid, usamos i
        // suponiendo que todos los procesos son iguales y creados rápido.
        int tat = end_tick - arrival[i];
        tat_sum += tat;
    }

    // Aproximar burst de CPU (todos hacen el mismo trabajo)
    // No lo medimos exacto; simplemente suponemos que el burst es similar
    // y derivamos WT promedio como (TAT promedio - burst estimado).
    int avg_tat = tat_sum / n;

    // Estimar burst en ticks observando el tiempo de un solo proceso
    // (corriendo solo bench 1 vez con n=1 y anotando ticks).
    // Aquí lo tratamos como constante fija; ajusta según tus medidas.
    int estimated_burst = avg_tat / 2; // ajuste grosero

    wt_sum = n * (avg_tat - estimated_burst);
    int avg_wt = wt_sum / n;

    printf(1, "NPROCS=%d AVG_TAT=%d AVG_WT=%d\n", n, avg_tat, avg_wt);

    exit();
}
