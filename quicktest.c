#include "types.h"
#include "stat.h"
#include "user.h"

// Test rápido para calibrar las iteraciones necesarias en tu sistema
int main(int argc, char *argv[]) {
    int start, end;
    int target_ticks = 50; // Queremos que cada test dure ~50 ticks
    
    printf(1, "\n=== CALIBRATION TEST ===\n");
    printf(1, "Finding optimal iteration counts for your system...\n\n");
    
    // Test 1: CPU work
    printf(1, "Testing CPU work with different iterations:\n");
    
    int cpu_iters[] = {1000, 3000, 5000, 10000, 15000};
    for (int i = 0; i < 5; i++) {
        start = uptime();
        
        int count = 0;
        for (int j = 0; j < cpu_iters[i]; j++) {
            // Trabajo CPU simple
            for (int k = 0; k < 1000; k++) {
                count += k % 7;
            }
        }
        
        end = uptime();
        int elapsed = end - start;
        
        printf(1, "  %d iterations: %d ticks", cpu_iters[i], elapsed);
        
        if (elapsed >= target_ticks - 10 && elapsed <= target_ticks + 10) {
            printf(1, " <-- GOOD!\n");
        } else if (elapsed < target_ticks - 10) {
            printf(1, " (too fast, increase)\n");
        } else {
            printf(1, " (too slow, decrease)\n");
        }
    }
    
    // Test 2: Prime calculation
    printf(1, "\nTesting prime calculation:\n");
    
    int prime_limits[] = {2000, 5000, 8000, 12000, 15000};
    for (int i = 0; i < 5; i++) {
        start = uptime();
        
        int prime_count = 0;
        for (int j = 2; j < prime_limits[i]; j++) {
            int is_prime = 1;
            if (j <= 1) is_prime = 0;
            else if (j <= 3) is_prime = 1;
            else if (j % 2 == 0 || j % 3 == 0) is_prime = 0;
            else {
                for (int k = 5; k * k <= j; k += 6) {
                    if (j % k == 0 || j % (k + 2) == 0) {
                        is_prime = 0;
                        break;
                    }
                }
            }
            if (is_prime) prime_count++;
        }
        
        end = uptime();
        int elapsed = end - start;
        
        printf(1, "  Up to %d: %d ticks (%d primes)", 
               prime_limits[i], elapsed, prime_count);
        
        if (elapsed >= target_ticks - 10 && elapsed <= target_ticks + 10) {
            printf(1, " <-- GOOD!\n");
        } else if (elapsed < target_ticks - 10) {
            printf(1, " (too fast)\n");
        } else {
            printf(1, " (too slow)\n");
        }
    }
    
    printf(1, "\n=== RECOMMENDATIONS ===\n");
    printf(1, "Use the iteration counts marked as 'GOOD' in benchrunner.c\n");
    printf(1, "Target: Each test should take 20-100 ticks for meaningful results\n");
    printf(1, "If all tests show 0 ticks, your system is too fast - increase iterations!\n");
    
    exit();
}