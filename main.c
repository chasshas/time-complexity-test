#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

// 어셈블리 함수 선언
extern void test_add();
extern void test_sub();
extern void test_mul();
extern void test_imul();
extern void test_div();
extern void test_mov();
extern void test_cmp();
extern void test_and();
extern void test_or();
extern void test_xor();
extern void test_shl();
extern void test_shr();
extern void test_lea();
extern void test_memory_load();
extern void test_memory_store();

#define ITERATIONS 10000000

typedef struct {
    const char* name;
    void (*func)();
} test_case_t;

static inline uint64_t rdtsc() {
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}

void flush_cache() {
    // 캐시 플러시를 위한 더미 작업
    volatile int dummy[1000000];
    for(int i = 0; i < 1000000; i++) {
        dummy[i] = i;
    }
}

int main() {
    test_case_t tests[] = {
        {"add", test_add},
        {"sub", test_sub},
        {"mul", test_mul},
        {"imul", test_imul},
        {"div", test_div},
        {"mov", test_mov},
        {"cmp", test_cmp},
        {"and", test_and},
        {"or", test_or},
        {"xor", test_xor},
        {"shl", test_shl},
        {"shr", test_shr},
        {"lea", test_lea},
        {"memory_load", test_memory_load},
        {"memory_store", test_memory_store},
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Assembly Instruction Benchmark\n");
    printf("Iterations per test: %d\n", ITERATIONS);
    printf("================================\n");

    for(int i = 0; i < num_tests; i++) {
        printf("Testing %s... ", tests[i].name);
        fflush(stdout);

        // 캐시 플러시
        flush_cache();

        // 웜업
        tests[i].func();

        // 실제 측정
        uint64_t start = rdtsc();
        tests[i].func();
        uint64_t end = rdtsc();

        uint64_t cycles = end - start;
        double cycles_per_op = (double)cycles / ITERATIONS;

        printf("%lu cycles (%.2f cycles/op)\n", cycles, cycles_per_op);
    }

    return 0;
}