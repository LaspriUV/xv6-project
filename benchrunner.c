#include "types.h"
#include "stat.h"
#include "user.h"

// Runner automático para ejecutar todos los benchmarks
void run_benchmark(char *name, char **args) {
    int pid = fork();
    
    if (pid < 0) {
        printf(1, "Fork failed for %s\n", name);
        return;
    }
    
    if (pid == 0) {
        exec(name, args);
        printf(1, "Exec failed for %s\n", name);
        exit();
    }
    
    wait();
}

int main(int argc, char *argv[]) {
    int start_tick, end_tick;
    
    printf(1, "\n========================================\n");
    printf(1, "XV6 BENCHMARK SUITE\n");
    printf(1, "========================================\n\n");
    
    start_tick = uptime();
    
    // Test 1: CPU Intensive (single process)
    printf(1, "\n--- Test 1: CPU Intensive (Single) ---\n");
    char *cpu_args[] = {"cpubench", "8000", 0};  // Aumentado
    run_benchmark("cpubench", cpu_args);
    
    // Test 2: I/O Intensive (single process)
    printf(1, "\n--- Test 2: I/O Intensive (Single) ---\n");
    char *io_args[] = {"iobench", "100", 0};  // Aumentado
    run_benchmark("iobench", io_args);
    
    // Test 3: Mixed workload
    printf(1, "\n--- Test 3: Mixed Workload ---\n");
    char *mixed_args[] = {"mixedbench", "80", 0};  // Aumentado
    run_benchmark("mixedbench", mixed_args);
    
    // Test 4: Fork benchmark (COW test - read-heavy)
    printf(1, "\n--- Test 4: Fork Benchmark (10%% writes - COW optimal) ---\n");
    char *fork1_args[] = {"forkbench", "8", "10", 0};  // Más forks
    run_benchmark("forkbench", fork1_args);
    
    // Test 5: Fork benchmark (write-heavy)
    printf(1, "\n--- Test 5: Fork Benchmark (80%% writes - COW stress) ---\n");
    char *fork2_args[] = {"forkbench", "8", "80", 0};  // Más forks
    run_benchmark("forkbench", fork2_args);
    
    // Test 6: Scheduler fairness
    printf(1, "\n--- Test 6: Scheduler Fairness (4 processes) ---\n");
    char *sched_args[] = {"schedulerbench", "4", "5000", 0};  // Más trabajo
    run_benchmark("schedulerbench", sched_args);
    
    // Test 7: Concurrent CPU benchmarks
    printf(1, "\n--- Test 7: Concurrent CPU Load (3 processes) ---\n");
    printf(1, "Starting 3 concurrent CPU benchmarks...\n");
    
    for (int i = 0; i < 3; i++) {
        int pid = fork();
        if (pid == 0) {
            char *concurrent_args[] = {"cpubench", "6000", 0};  // Aumentado
            exec("cpubench", concurrent_args);
            exit();
        }
    }
    
    // Esperar todos los procesos concurrentes
    for (int i = 0; i < 3; i++) {
        wait();
    }
    
    end_tick = uptime();
    
    printf(1, "\n========================================\n");
    printf(1, "BENCHMARK SUITE COMPLETED\n");
    printf(1, "Total time: %d ticks\n", end_tick - start_tick);
    printf(1, "========================================\n\n");
    
    exit();
}