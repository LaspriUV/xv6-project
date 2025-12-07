#include "types.h"
#include "stat.h"
#include "user.h"

#define ARRAY_SIZE 512  // Reducido para evitar stack overflow en xv6

// Benchmark para COW: muchos forks con diferentes patrones de escritura
int main(int argc, char *argv[]) {
    int num_forks = 5;
    int write_percent = 10; // Porcentaje de memoria a escribir
    int start_tick, end_tick;
    static int array[ARRAY_SIZE];  // static para usar data segment, no stack
    
    if (argc > 1) {
        num_forks = atoi(argv[1]);
    }
    if (argc > 2) {
        write_percent = atoi(argv[2]);
    }
    
    // Inicializar array antes de fork
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i;
    }
    
    printf(1, "[FORK] Parent PID %d: Starting fork benchmark (%d forks, %d%% writes)...\n", 
           getpid(), num_forks, write_percent);
    
    start_tick = uptime();
    
    for (int i = 0; i < num_forks; i++) {
        int pid = fork();
        
        if (pid < 0) {
            printf(1, "[FORK] Parent: Fork failed\n");
            exit();
        }
        
        if (pid == 0) {
            // Proceso hijo
            int child_start = uptime();
            int sum = 0;
            
            // Leer todo el array (debería ser rápido con COW)
            for (int j = 0; j < ARRAY_SIZE; j++) {
                sum += array[j];
            }
            
            // Escribir solo un porcentaje (trigger COW)
            int writes = (ARRAY_SIZE * write_percent) / 100;
            for (int j = 0; j < writes; j++) {
                array[j] = array[j] * 2;
            }
            
            // Hacer algo de trabajo CPU
            for (int j = 0; j < 1000; j++) {
                sum += j;
            }
            
            int child_end = uptime();
            
            printf(1, "[FORK] Child PID %d (fork %d): sum=%d, completed in %d ticks\n", 
                   getpid(), i, sum, child_end - child_start);
            
            exit();
        }
    }
    
    // Padre espera a todos los hijos
    for (int i = 0; i < num_forks; i++) {
        wait();
    }
    
    end_tick = uptime();
    
    printf(1, "[FORK] Parent PID %d: All %d children completed in %d ticks\n", 
           getpid(), num_forks, end_tick - start_tick);
    printf(1, "[FORK] Parent PID %d: COMPLETED in %d ticks\n", 
           getpid(), end_tick - start_tick);
    
    exit();
}