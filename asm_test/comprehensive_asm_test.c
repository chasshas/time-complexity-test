#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <emmintrin.h>  // SSE2
#include <immintrin.h>  // AVX
#include <x86intrin.h>  // 추가 intrinsics

#define ITERATIONS 50000000
#define WARMUP_ITERATIONS 5000000
#define ENERGY_MEASUREMENT_DELAY_MS 100
#define TEST_COUNT 25

// MSR 레지스터 정의 (AMD Ryzen 전력 측정용)
#define MSR_PWR_UNIT        0xC0010299
#define MSR_CORE_ENERGY     0xC001029A
#define MSR_PKG_ENERGY      0xC001029B

typedef struct {
    char name[64];
    double avg_cycles;
    double avg_time_ns;
    double energy_start;
    double energy_end;
    double energy_consumed;
} test_result_t;

// RDTSC를 이용한 정확한 사이클 측정
static inline uint64_t rdtsc(void) {
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

// MSR 읽기 함수
uint64_t read_msr(int cpu, uint32_t reg) {
    char path[32];
    int fd;
    uint64_t value;
    
    snprintf(path, sizeof(path), "/dev/cpu/%d/msr", cpu);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return 0;
    }
    
    if (pread(fd, &value, sizeof(value), reg) != sizeof(value)) {
        close(fd);
        return 0;
    }
    
    close(fd);
    return value;
}

// 전력 측정 함수
double get_energy_joules(int cpu) {
    uint64_t energy_raw = read_msr(cpu, MSR_CORE_ENERGY);
    uint64_t unit_raw = read_msr(cpu, MSR_PWR_UNIT);
    
    if (energy_raw == 0 || unit_raw == 0) {
        return 0.0;
    }
    
    double energy_unit = 1.0 / (1 << ((unit_raw >> 8) & 0x1F));
    return energy_raw * energy_unit;
}

// ============ 기본 산술 명령어 테스트 ============

void test_add_instruction(test_result_t *result) {
    strcpy(result->name, "ADD (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 1, b = 2, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("addl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("addl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void test_sub_instruction(test_result_t *result) {
    strcpy(result->name, "SUB (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 100, b = 1, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("subl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("subl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void test_mul_instruction(test_result_t *result) {
    strcpy(result->name, "IMUL (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 123, b = 456, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("imull %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("imull %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void test_div_instruction(test_result_t *result) {
    strcpy(result->name, "DIV (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 1000000, b = 7, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("movl %1, %%eax; xorl %%edx, %%edx; divl %2; movl %%eax, %0"
                         : "=r"(c) : "r"(a), "r"(b) : "eax", "edx");
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("movl %1, %%eax; xorl %%edx, %%edx; divl %2; movl %%eax, %0"
                         : "=r"(c) : "r"(a), "r"(b) : "eax", "edx");
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

// ============ 논리 연산 명령어 테스트 ============

void test_and_instruction(test_result_t *result) {
    strcpy(result->name, "AND (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 0xAAAAAAAA, b = 0x55555555, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("andl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("andl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void test_or_instruction(test_result_t *result) {
    strcpy(result->name, "OR (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 0xAAAAAAAA, b = 0x55555555, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("orl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("orl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void test_xor_instruction(test_result_t *result) {
    strcpy(result->name, "XOR (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 0xAAAAAAAA, b = 0x55555555, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("xorl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("xorl %1, %0" : "=r"(c) : "r"(b), "0"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

// ============ 시프트 연산 명령어 테스트 ============

void test_shl_instruction(test_result_t *result) {
    strcpy(result->name, "SHL (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 0x12345678, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("shll $4, %0" : "=r"(c) : "0"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("shll $4, %0" : "=r"(c) : "0"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void test_shr_instruction(test_result_t *result) {
    strcpy(result->name, "SHR (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 0x87654321, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("shrl $4, %0" : "=r"(c) : "0"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("shrl $4, %0" : "=r"(c) : "0"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

// ============ 메모리 이동 명령어 테스트 ============

void test_mov_instruction(test_result_t *result) {
    strcpy(result->name, "MOV (register)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 0x12345678, b;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("movl %1, %0" : "=r"(b) : "r"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("movl %1, %0" : "=r"(b) : "r"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void test_cmp_instruction(test_result_t *result) {
    strcpy(result->name, "CMP (32-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint32_t a = 100, b = 200;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("cmpl %0, %1" : : "r"(a), "r"(b) : "cc");
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("cmpl %0, %1" : : "r"(a), "r"(b) : "cc");
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

// ============ SSE/SSE2 명령어 테스트 ============

void test_sse_add_instruction(test_result_t *result) {
    strcpy(result->name, "SSE2 PADDQ");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    uint64_t test_data[4] = {0x1111111111111111ULL, 0x2222222222222222ULL,
                             0x3333333333333333ULL, 0x4444444444444444ULL};
    volatile uint64_t dummy_result = 0;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile (
            "movq %1, %%xmm0\n\t"
            "movq %2, %%xmm1\n\t"
            "movq %3, %%xmm2\n\t" 
            "movq %4, %%xmm3\n\t"
            "punpcklqdq %%xmm1, %%xmm0\n\t"
            "punpcklqdq %%xmm3, %%xmm2\n\t"
            "paddq %%xmm2, %%xmm0\n\t"
            "movq %%xmm0, %0\n\t"
            : "=m"(dummy_result)
            : "m"(test_data[0]), "m"(test_data[1]), "m"(test_data[2]), "m"(test_data[3])
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile (
            "movq %1, %%xmm0\n\t"
            "movq %2, %%xmm1\n\t"
            "movq %3, %%xmm2\n\t"
            "movq %4, %%xmm3\n\t"
            "punpcklqdq %%xmm1, %%xmm0\n\t"
            "punpcklqdq %%xmm3, %%xmm2\n\t"
            "paddq %%xmm2, %%xmm0\n\t"
            "movq %%xmm0, %0\n\t"
            : "=m"(dummy_result)
            : "m"(test_data[0]), "m"(test_data[1]), "m"(test_data[2]), "m"(test_data[3])
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    if (dummy_result == 0x123456789ABCDEF0ULL) printf("");
}

void test_sse_float_add_instruction(test_result_t *result) {
    strcpy(result->name, "SSE ADDPS");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    float test_data[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    volatile float dummy_result = 0;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile (
            "movups %1, %%xmm0\n\t"
            "movups %2, %%xmm1\n\t"
            "addps %%xmm1, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m"(dummy_result)
            : "m"(test_data[0]), "m"(test_data[4])
            : "xmm0", "xmm1", "memory"
        );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile (
            "movups %1, %%xmm0\n\t"
            "movups %2, %%xmm1\n\t"
            "addps %%xmm1, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m"(dummy_result)
            : "m"(test_data[0]), "m"(test_data[4])
            : "xmm0", "xmm1", "memory"
        );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    if (dummy_result == 123.456f) printf("");
}

void test_sse_float_mul_instruction(test_result_t *result) {
    strcpy(result->name, "SSE MULPS");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    float test_data[8] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f};
    volatile float dummy_result = 0;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile (
            "movups %1, %%xmm0\n\t"
            "movups %2, %%xmm1\n\t"
            "mulps %%xmm1, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m"(dummy_result)
            : "m"(test_data[0]), "m"(test_data[4])
            : "xmm0", "xmm1", "memory"
        );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile (
            "movups %1, %%xmm0\n\t"
            "movups %2, %%xmm1\n\t"
            "mulps %%xmm1, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m"(dummy_result)
            : "m"(test_data[0]), "m"(test_data[4])
            : "xmm0", "xmm1", "memory"
        );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    if (dummy_result == 123.456f) printf("");
}

// ============ AVX/AVX2 명령어 테스트 ============

void test_avx_add_instruction(test_result_t *result) {
    strcpy(result->name, "AVX2 VPADDQ");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    uint64_t test_data_a[4] = {0x1111111111111111ULL, 0x2222222222222222ULL,
                               0x3333333333333333ULL, 0x4444444444444444ULL};
    uint64_t test_data_b[4] = {0x5555555555555555ULL, 0x6666666666666666ULL,
                               0x7777777777777777ULL, 0x8888888888888888ULL};
    volatile uint64_t dummy_result = 0;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile (
            "vmovdqu %1, %%ymm0\n\t"
            "vmovdqu %2, %%ymm1\n\t"
            "vpaddq %%ymm1, %%ymm0, %%ymm0\n\t"
            "vmovq %%xmm0, %0\n\t"
            "vzeroupper\n\t"
            : "=m"(dummy_result)
            : "m"(test_data_a[0]), "m"(test_data_b[0])
            : "ymm0", "ymm1", "memory"
        );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile (
            "vmovdqu %1, %%ymm0\n\t"
            "vmovdqu %2, %%ymm1\n\t"
            "vpaddq %%ymm1, %%ymm0, %%ymm0\n\t"
            "vmovq %%xmm0, %0\n\t"
            "vzeroupper\n\t"
            : "=m"(dummy_result)
            : "m"(test_data_a[0]), "m"(test_data_b[0])
            : "ymm0", "ymm1", "memory"
        );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    if (dummy_result == 0x123456789ABCDEF0ULL) printf("");
}

void test_avx_float_add_instruction(test_result_t *result) {
    strcpy(result->name, "AVX VADDPS");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    float test_data_a[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float test_data_b[8] = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    volatile float dummy_result = 0;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile (
            "vmovups %1, %%ymm0\n\t"
            "vmovups %2, %%ymm1\n\t"
            "vaddps %%ymm1, %%ymm0, %%ymm0\n\t"
            "vmovss %%xmm0, %0\n\t"
            "vzeroupper\n\t"
            : "=m"(dummy_result)
            : "m"(test_data_a[0]), "m"(test_data_b[0])
            : "ymm0", "ymm1", "memory"
        );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile (
            "vmovups %1, %%ymm0\n\t"
            "vmovups %2, %%ymm1\n\t"
            "vaddps %%ymm1, %%ymm0, %%ymm0\n\t"
            "vmovss %%xmm0, %0\n\t"
            "vzeroupper\n\t"
            : "=m"(dummy_result)
            : "m"(test_data_a[0]), "m"(test_data_b[0])
            : "ymm0", "ymm1", "memory"
        );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    if (dummy_result == 123.456f) printf("");
}

void test_avx_float_mul_instruction(test_result_t *result) {
    strcpy(result->name, "AVX VMULPS");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    float test_data_a[8] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f};
    float test_data_b[8] = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    volatile float dummy_result = 0;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile (
            "vmovups %1, %%ymm0\n\t"
            "vmovups %2, %%ymm1\n\t"
            "vmulps %%ymm1, %%ymm0, %%ymm0\n\t"
            "vmovss %%xmm0, %0\n\t"
            "vzeroupper\n\t"
            : "=m"(dummy_result)
            : "m"(test_data_a[0]), "m"(test_data_b[0])
            : "ymm0", "ymm1", "memory"
        );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile (
            "vmovups %1, %%ymm0\n\t"
            "vmovups %2, %%ymm1\n\t"
            "vmulps %%ymm1, %%ymm0, %%ymm0\n\t"
            "vmovss %%xmm0, %0\n\t"
            "vzeroupper\n\t"
            : "=m"(dummy_result)
            : "m"(test_data_a[0]), "m"(test_data_b[0])
            : "ymm0", "ymm1", "memory"
        );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    if (dummy_result == 123.456f) printf("");
}

// ============ 비트 조작 명령어 테스트 ============

void test_popcnt_instruction(test_result_t *result) {
    strcpy(result->name, "POPCNT (64-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint64_t a = 0x123456789ABCDEFULL, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("popcntq %1, %0" : "=r"(c) : "r"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("popcntq %1, %0" : "=r"(c) : "r"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void test_lzcnt_instruction(test_result_t *result) {
    strcpy(result->name, "LZCNT (64-bit)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile uint64_t a = 0x0000123456789ABCULL, c;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile ("lzcntq %1, %0" : "=r"(c) : "r"(a) : );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile ("lzcntq %1, %0" : "=r"(c) : "r"(a) : );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

// ============ 메모리 접근 명령어 테스트 ============

void test_memory_load(test_result_t *result) {
    strcpy(result->name, "Memory LOAD (L1)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    // 32KB 배열 (L1 캐시 크기)
    volatile uint32_t *array = malloc(32 * 1024);
    for (int i = 0; i < 8 * 1024; i++) {
        array[i] = i;
    }
    
    volatile uint32_t sum = 0;
    int iterations = ITERATIONS / 1000;
    
    for (int i = 0; i < iterations / 10; i++) {
        sum += array[i % (8 * 1024)];
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < iterations; i++) {
        sum += array[i % (8 * 1024)];
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / iterations;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / iterations;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    free((void*)array);
}

void test_memory_load_l2(test_result_t *result) {
    strcpy(result->name, "Memory LOAD (L2)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    // 256KB 배열 (L2 캐시 크기)
    volatile uint32_t *array = malloc(256 * 1024);
    for (int i = 0; i < 64 * 1024; i++) {
        array[i] = i;
    }
    
    volatile uint32_t sum = 0;
    int iterations = ITERATIONS / 1000;
    
    for (int i = 0; i < iterations / 10; i++) {
        sum += array[i % (64 * 1024)];
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < iterations; i++) {
        sum += array[i % (64 * 1024)];
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / iterations;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / iterations;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    free((void*)array);
}

void test_memory_load_l3(test_result_t *result) {
    strcpy(result->name, "Memory LOAD (L3)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    // 8MB 배열 (L3 캐시 크기)
    volatile uint32_t *array = malloc(8 * 1024 * 1024);
    for (int i = 0; i < 2 * 1024 * 1024; i++) {
        array[i] = i;
    }
    
    volatile uint32_t sum = 0;
    int iterations = ITERATIONS / 1000;
    
    for (int i = 0; i < iterations / 10; i++) {
        sum += array[i % (2 * 1024 * 1024)];
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < iterations; i++) {
        sum += array[i % (2 * 1024 * 1024)];
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / iterations;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / iterations;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    free((void*)array);
}

void test_memory_load_ram(test_result_t *result) {
    strcpy(result->name, "Memory LOAD (RAM)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    
    // 64MB 배열 (RAM 접근)
    volatile uint32_t *array = malloc(64 * 1024 * 1024);
    for (int i = 0; i < 16 * 1024 * 1024; i++) {
        array[i] = i;
    }
    
    volatile uint32_t sum = 0;
    int iterations = ITERATIONS / 10000; // RAM 접근은 더 적은 반복
    
    for (int i = 0; i < iterations / 10; i++) {
        sum += array[(i * 1024) % (16 * 1024 * 1024)]; // 스트라이드 접근으로 캐시 미스 유발
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < iterations; i++) {
        sum += array[(i * 1024) % (16 * 1024 * 1024)];
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / iterations;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / iterations;
    result->energy_consumed = result->energy_end - result->energy_start;
    
    free((void*)array);
}

// ============ 분기 명령어 테스트 ============

void test_branch_instruction(test_result_t *result) {
    strcpy(result->name, "Branch (taken)");
    
    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    volatile int counter = 0;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        __asm__ volatile (
            "cmpl $0, %0\n\t"
            "je 1f\n\t"
            "incl %0\n\t"
            "1:\n\t"
            : "+r"(counter)
            :
            : "cc"
        );
    }
    
    result->energy_start = get_energy_joules(0);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtsc();
    
    for (int i = 0; i < ITERATIONS; i++) {
        __asm__ volatile (
            "cmpl $0, %0\n\t"
            "je 1f\n\t"
            "incl %0\n\t"
            "1:\n\t"
            : "+r"(counter)
            :
            : "cc"
        );
    }
    
    end_cycles = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    result->energy_end = get_energy_joules(0);
    
    result->avg_cycles = (double)(end_cycles - start_cycles) / ITERATIONS;
    result->avg_time_ns = ((end_time.tv_sec - start_time.tv_sec) * 1e9 + 
                          (end_time.tv_nsec - start_time.tv_nsec)) / ITERATIONS;
    result->energy_consumed = result->energy_end - result->energy_start;
}

void run_comprehensive_test_suite() {
    test_result_t results[TEST_COUNT];
    
    printf("AMD Ryzen 5 5600 Comprehensive Assembly Performance Test\n");
    printf("=========================================================\n\n");
    
    printf("Testing %d iterations per instruction...\n\n", ITERATIONS);
    
    // 기본 산술 명령어들
    printf("Running Basic Arithmetic Instructions...\n");
    test_add_instruction(&results[0]);
    test_sub_instruction(&results[1]);
    test_mul_instruction(&results[2]);
    test_div_instruction(&results[3]);
    
    // 논리 연산 명령어들
    printf("Running Logical Instructions...\n");
    test_and_instruction(&results[4]);
    test_or_instruction(&results[5]);
    test_xor_instruction(&results[6]);
    
    // 시프트 연산 명령어들
    printf("Running Shift Instructions...\n");
    test_shl_instruction(&results[7]);
    test_shr_instruction(&results[8]);
    
    // 기본 명령어들
    printf("Running Basic Instructions...\n");
    test_mov_instruction(&results[9]);
    test_cmp_instruction(&results[10]);
    
    // SSE/SSE2 명령어들
    printf("Running SSE Instructions...\n");
    test_sse_add_instruction(&results[11]);
    test_sse_float_add_instruction(&results[12]);
    test_sse_float_mul_instruction(&results[13]);
    
    // AVX/AVX2 명령어들
    printf("Running AVX Instructions...\n");
    test_avx_add_instruction(&results[14]);
    test_avx_float_add_instruction(&results[15]);
    test_avx_float_mul_instruction(&results[16]);
    
    // 비트 조작 명령어들
    printf("Running Bit Manipulation Instructions...\n");
    test_popcnt_instruction(&results[17]);
    test_lzcnt_instruction(&results[18]);
    
    // 메모리 접근 명령어들
    printf("Running Memory Access Instructions...\n");
    test_memory_load(&results[19]);
    test_memory_load_l2(&results[20]);
    test_memory_load_l3(&results[21]);
    test_memory_load_ram(&results[22]);
    
    // 분기 명령어들
    printf("Running Branch Instructions...\n");
    test_branch_instruction(&results[23]);
    
    // 결과 출력
    printf("\nComprehensive Results:\n");
    printf("%-25s %12s %12s %12s\n", "Instruction", "Cycles", "Time(ns)", "Energy(J)");
    printf("---------------------------------------------------------------------\n");
    
    for (int i = 0; i < TEST_COUNT - 1; i++) {
        printf("%-25s %12.3f %12.3f %12.6f\n", 
               results[i].name, 
               results[i].avg_cycles,
               results[i].avg_time_ns,
               results[i].energy_consumed);
    }
    
    printf("\nCache Hierarchy Analysis:\n");
    printf("L1 Cache Access: %.3f cycles\n", results[19].avg_cycles);
    printf("L2 Cache Access: %.3f cycles\n", results[20].avg_cycles);
    printf("L3 Cache Access: %.3f cycles\n", results[21].avg_cycles);
    printf("RAM Access:      %.3f cycles\n", results[22].avg_cycles);
    
    printf("\nInstruction Categories Performance:\n");
    printf("Basic Arithmetic Avg: %.3f cycles\n", 
           (results[0].avg_cycles + results[1].avg_cycles + results[2].avg_cycles + results[3].avg_cycles) / 4);
    printf("Logical Operations Avg: %.3f cycles\n",
           (results[4].avg_cycles + results[5].avg_cycles + results[6].avg_cycles) / 3);
    printf("SIMD Integer Avg: %.3f cycles\n",
           (results[11].avg_cycles + results[14].avg_cycles) / 2);
    printf("SIMD Float Avg: %.3f cycles\n",
           (results[12].avg_cycles + results[13].avg_cycles + results[15].avg_cycles + results[16].avg_cycles) / 4);
    
    printf("\nNotes:\n");
    printf("- Energy measurement requires MSR access (run as root)\n");
    printf("- Results may vary depending on system load and frequency scaling\n");
    printf("- Disable CPU frequency scaling for more consistent results\n");
    printf("- Cache measurements show memory hierarchy performance\n");
}

int main() {
    // CPU 정보 확인
    system("echo 'CPU Info:' && cat /proc/cpuinfo | grep 'model name' | head -1");
    printf("\n");
    
    run_comprehensive_test_suite();
    
    return 0;
}
