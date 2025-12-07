#include "types.h"
#include "stat.h"
#include "user.h"

// Benchmark CPU-intensive: calcula números primos
int is_prime(int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int iterations = 5000; // Ajustable según tu hardware
    int prime_count = 0;
    int start_tick, end_tick;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    printf(1, "[CPU] PID %d: Starting CPU-intensive benchmark (%d iterations)...\n", 
           getpid(), iterations);
    
    start_tick = uptime();
    
    // Trabajo CPU-intensive: buscar primos
    for (int i = 2; i < iterations; i++) {
        if (is_prime(i)) {
            prime_count++;
        }
    }
    
    end_tick = uptime();
    
    printf(1, "[CPU] PID %d: Found %d primes in %d ticks\n", 
           getpid(), prime_count, end_tick - start_tick);
    printf(1, "[CPU] PID %d: COMPLETED in %d ticks\n", 
           getpid(), end_tick - start_tick);
    
    exit();
}