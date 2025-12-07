#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

// Benchmark mixto: alterna CPU e I/O
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
    int iterations = 50;
    int fd;
    char buf[128];
    int start_tick, end_tick;
    char filename[20];
    int prime_count = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    strcpy(filename, "mixed_");
    filename[6] = '0' + (getpid() % 10);
    filename[7] = '\0';
    
    printf(1, "[MIXED] PID %d: Starting mixed benchmark (%d iterations)...\n", 
           getpid(), iterations);
    
    start_tick = uptime();
    
    for (int i = 0; i < iterations; i++) {
        // Fase 1: Trabajo CPU
        for (int j = i * 10; j < i * 10 + 100; j++) {
            if (is_prime(j)) {
                prime_count++;
            }
        }
        
        // Fase 2: Trabajo I/O
        fd = open(filename, O_CREATE | O_WRONLY);
        if (fd >= 0) {
            memset(buf, 'A' + (i % 26), sizeof(buf));
            write(fd, buf, sizeof(buf));
            close(fd);
        }
        
        // Fase 3: Más CPU
        int sum = 0;
        for (int j = 0; j < 100; j++) {
            sum += j * j;
        }
        
        // Fase 4: Leer archivo
        fd = open(filename, O_RDONLY);
        if (fd >= 0) {
            read(fd, buf, sizeof(buf));
            close(fd);
        }
    }
    
    unlink(filename);
    
    end_tick = uptime();
    
    printf(1, "[MIXED] PID %d: Found %d primes, completed in %d ticks\n", 
           getpid(), prime_count, end_tick - start_tick);
    printf(1, "[MIXED] PID %d: COMPLETED in %d ticks\n", 
           getpid(), end_tick - start_tick);
    
    exit();
}