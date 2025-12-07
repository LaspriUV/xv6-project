#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

// Benchmark I/O-intensive: muchas operaciones de lectura/escritura
int main(int argc, char *argv[]) {
    int iterations = 100;
    int fd;
    char buf[512];
    int start_tick, end_tick;
    char filename[20];
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    // Usar PID para nombre único de archivo
    strcpy(filename, "iobench_");
    filename[8] = '0' + (getpid() % 10);
    filename[9] = '\0';
    
    printf(1, "[I/O] PID %d: Starting I/O-intensive benchmark (%d iterations)...\n", 
           getpid(), iterations);
    
    start_tick = uptime();
    
    // Trabajo I/O-intensive: escribir y leer archivo repetidamente
    for (int i = 0; i < iterations; i++) {
        // Escribir
        fd = open(filename, O_CREATE | O_WRONLY);
        if (fd < 0) {
            printf(1, "[I/O] PID %d: ERROR opening file for write\n", getpid());
            exit();
        }
        
        memset(buf, 'A' + (i % 26), sizeof(buf));
        write(fd, buf, sizeof(buf));
        close(fd);
        
        // Leer
        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            printf(1, "[I/O] PID %d: ERROR opening file for read\n", getpid());
            exit();
        }
        
        read(fd, buf, sizeof(buf));
        close(fd);
    }
    
    // Limpiar
    unlink(filename);
    
    end_tick = uptime();
    
    printf(1, "[I/O] PID %d: Completed %d I/O operations in %d ticks\n", 
           getpid(), iterations * 2, end_tick - start_tick);
    printf(1, "[I/O] PID %d: COMPLETED in %d ticks\n", 
           getpid(), end_tick - start_tick);
    
    exit();
}