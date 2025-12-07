#include "types.h"
#include "stat.h"
#include "user.h"

// Test de fairness: múltiples procesos idénticos compitiendo por CPU
int main(int argc, char *argv[]) {
    int num_procs = 4;
    int work_amount = 5000;  // Aumentado para que tome más tiempo
    int start_tick, end_tick;
    
    if (argc > 1) {
        num_procs = atoi(argv[1]);
    }
    if (argc > 2) {
        work_amount = atoi(argv[2]);
    }
    
    printf(1, "[SCHED] Parent PID %d: Starting scheduler fairness test (%d processes)...\n", 
           getpid(), num_procs);
    
    start_tick = uptime();
    
    for (int i = 0; i < num_procs; i++) {
        int pid = fork();
        
        if (pid < 0) {
            printf(1, "[SCHED] Fork failed\n");
            exit();
        }
        
        if (pid == 0) {
            // Cada hijo hace la misma cantidad de trabajo
            int child_start = uptime();
            int count = 0;
            
            // Trabajo CPU puro
            for (int j = 0; j < work_amount; j++) {
                for (int k = 0; k < 1000; k++) {
                    count += k % 7;
                }
            }
            
            int child_end = uptime();
            int elapsed = child_end - child_start;
            
            printf(1, "[SCHED] Child %d PID %d: work=%d, time=%d ticks, ticks/work=", 
                   i, getpid(), count, elapsed);
            
            // Calcular eficiencia (menor es mejor si todos son iguales = fair)
            if (work_amount > 0) {
                printf(1, "%d\n", elapsed * 100 / work_amount);
            } else {
                printf(1, "N/A\n");
            }
            
            printf(1, "[SCHED] Child %d PID %d: COMPLETED in %d ticks\n", 
                   i, getpid(), elapsed);
            
            exit();
        }
    }
    
    // Padre espera a todos
    for (int i = 0; i < num_procs; i++) {
        wait();
    }
    
    end_tick = uptime();
    
    printf(1, "[SCHED] Parent: All processes completed in %d ticks total\n", 
           end_tick - start_tick);
    printf(1, "[SCHED] Parent: Average time per process: %d ticks\n", 
           (end_tick - start_tick) / num_procs);
    
    exit();
}