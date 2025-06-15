#include <stdio.h>
#include <time.h>
#include <stdint.h>

extern void test_add();

static inline uint64_t rdtsc() {
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}

int main() {
    printf("Manual Assembly Benchmark Test\n");
    printf("==============================\n");
    
    struct timespec start, end;
    
    // 측정
    clock_gettime(CLOCK_MONOTONIC, &start);
    uint64_t start_cycles = rdtsc();
    
    test_add();
    
    uint64_t end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // 결과 계산
    double time_taken = (end.tv_sec - start.tv_sec) + 
                       (end.tv_nsec - start.tv_nsec) / 1e9;
    uint64_t cycles = end_cycles - start_cycles;
    
    printf("Time: %.3f seconds\n", time_taken);
    printf("Cycles: %lu\n", cycles);
    printf("Frequency: %.2f GHz\n", cycles / (time_taken * 1e9));
    
    return 0;
}
