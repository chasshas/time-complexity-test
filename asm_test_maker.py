#!/usr/bin/env python3
import random
import os
import subprocess
import time


class AssemblyBenchmarkGenerator:
    def __init__(self, data_size=100000, iterations=1000000):
        self.data_size = data_size
        self.iterations = iterations
        self.random_data = []
        self.random_addresses = []

    def generate_random_data(self):
        """캐시 미스를 위한 랜덤 데이터 생성"""
        print(f"Generating {self.data_size} random values...")
        self.random_data = [random.randint(1, 0xFFFFFFFF) for _ in range(self.data_size)]

        # 메모리 접근 패턴을 예측 불가능하게 만들기 위한 랜덤 인덱스
        self.random_addresses = [random.randint(0, self.data_size - 1) for _ in range(self.iterations)]

    def create_data_section(self):
        """데이터 섹션 생성"""
        data_section = ".section .data\n"
        data_section += "    .align 64\n"  # 캐시 라인 정렬
        data_section += "test_data:\n"

        # 8바이트씩 묶어서 .quad로 정의
        for i in range(0, len(self.random_data), 2):
            if i + 1 < len(self.random_data):
                val1 = self.random_data[i] & 0xFFFFFFFF
                val2 = self.random_data[i + 1] & 0xFFFFFFFF
                combined = (val2 << 32) | val1
                data_section += f"    .quad 0x{combined:016x}\n"
            else:
                val = self.random_data[i] & 0xFFFFFFFF
                data_section += f"    .quad 0x{val:016x}\n"

        data_section += "\nrandom_indices:\n"
        for i in range(0, len(self.random_addresses), 8):
            indices = self.random_addresses[i:i + 8]
            while len(indices) < 8:
                indices.append(0)
            data_section += "    .quad " + ", ".join(str(idx) for idx in indices) + "\n"

        data_section += f"\ndata_size: .quad {self.data_size}\n"
        data_section += f"iterations: .quad {self.iterations}\n"

        return data_section

    def create_instruction_test(self, instruction_name, asm_code, setup_code="", cleanup_code=""):
        """특정 명령어 테스트 함수 생성"""
        return f"""
.global test_{instruction_name}
test_{instruction_name}:
    push %rbp
    mov %rsp, %rbp
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12

    # 기본 설정
    mov iterations(%rip), %rcx
    lea test_data(%rip), %rsi
    lea random_indices(%rip), %rdi
    xor %r8, %r8                    # 인덱스 카운터

    {setup_code}

test_{instruction_name}_loop:
    # 랜덤 메모리 접근으로 캐시 무력화
    mov (%rdi,%r8,8), %rax          # 랜덤 인덱스 로드
    and $0x{(self.data_size - 1):x}, %rax    # 범위 제한
    mov (%rsi,%rax,8), %rdx         # 랜덤 데이터 로드

    # 실제 테스트할 명령어들
{asm_code}

    # 다음 반복
    inc %r8
    and $0x{(len(self.random_addresses) - 1):x}, %r8  # 인덱스 순환
    dec %rcx
    jnz test_{instruction_name}_loop

    {cleanup_code}

    # 레지스터 복원
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rbp
    ret
"""

    def generate_assembly_file(self):
        """전체 어셈블리 파일 생성"""
        asm_content = self.create_data_section()
        asm_content += "\n.section .text\n"

        # 다양한 명령어 테스트들
        tests = {
            "add": {
                "code": "    add %rdx, %rax\n    add $1, %rax",
                "setup": "xor %rax, %rax"
            },
            "sub": {
                "code": "    sub %rdx, %rax\n    sub $1, %rax",
                "setup": "mov $0xFFFFFFFF, %rax"
            },
            "mul": {
                "code": "    mov %rdx, %rax\n    mul %rdx",
                "setup": "",
                "cleanup": "# mul result in rdx:rax"
            },
            "imul": {
                "code": "    imul %rdx, %rax",
                "setup": "mov %rdx, %rax"
            },
            "div": {
                "code": "    xor %rdx, %rdx\n    div %rbx",
                "setup": "mov $0xFFFFFFFF, %rax\n    mov $0x12345, %rbx\n    or $1, %rbx"
            },
            "mov": {
                "code": "    mov %rdx, %rax\n    mov %rax, %rbx\n    mov %rbx, %r9",
                "setup": ""
            },
            "cmp": {
                "code": "    cmp %rdx, %rax\n    cmp $0x12345, %rax",
                "setup": "mov %rdx, %rax"
            },
            "and": {
                "code": "    and %rdx, %rax\n    and $0xFFFF, %rax",
                "setup": "mov %rdx, %rax"
            },
            "or": {
                "code": "    or %rdx, %rax\n    or $0xF0F0, %rax",
                "setup": "mov %rdx, %rax"
            },
            "xor": {
                "code": "    xor %rdx, %rax\n    xor $0xAAAA, %rax",
                "setup": "mov %rdx, %rax"
            },
            "shl": {
                "code": "    shl $1, %rax\n    shl $2, %rax",
                "setup": "mov %rdx, %rax"
            },
            "shr": {
                "code": "    shr $1, %rax\n    shr $2, %rax",
                "setup": "mov %rdx, %rax"
            },
            "lea": {
                "code": "    lea (%rsi,%rdx,2), %rax\n    lea 8(%rax), %rbx",
                "setup": ""
            },
            "memory_load": {
                "code": "    mov (%rsi,%rax,8), %rbx\n    mov 8(%rsi,%rax,8), %r9",
                "setup": ""
            },
            "memory_store": {
                "code": "    mov %rdx, (%rsi,%rax,8)\n    mov %rax, 8(%rsi,%rax,8)",
                "setup": ""
            }
        }

        for name, test in tests.items():
            asm_content += self.create_instruction_test(
                name,
                test["code"],
                test.get("setup", ""),
                test.get("cleanup", "")
            )

        return asm_content

    def create_c_driver(self):
        """C 드라이버 코드 생성"""
        c_code = '''#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

// 어셈블리 함수 선언
'''

        instructions = ["add", "sub", "mul", "imul", "div", "mov", "cmp", "and", "or", "xor", "shl", "shr", "lea",
                        "memory_load", "memory_store"]

        for instr in instructions:
            c_code += f"extern void test_{instr}();\n"

        c_code += f'''
#define ITERATIONS {self.iterations}

typedef struct {{
    const char* name;
    void (*func)();
}} test_case_t;

static inline uint64_t rdtsc() {{
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}}

void flush_cache() {{
    // 캐시 플러시를 위한 더미 작업
    volatile int dummy[1000000];
    for(int i = 0; i < 1000000; i++) {{
        dummy[i] = i;
    }}
}}

int main() {{
    test_case_t tests[] = {{
'''

        for instr in instructions:
            c_code += f'        {{"{instr}", test_{instr}}},\n'

        c_code += '''    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Assembly Instruction Benchmark\\n");
    printf("Iterations per test: %d\\n", ITERATIONS);
    printf("================================\\n");

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

        printf("%lu cycles (%.2f cycles/op)\\n", cycles, cycles_per_op);
    }

    return 0;
}'''
        return c_code

    def create_makefile(self):
        """Makefile 생성"""
        makefile = '''CC = gcc
AS = as
CFLAGS = -O2 -march=native
ASFLAGS = -64

TARGET = benchmark
ASM_SRC = benchmark.s
C_SRC = main.c
ASM_OBJ = benchmark.o
C_OBJ = main.o

all: $(TARGET)

$(TARGET): $(ASM_OBJ) $(C_OBJ)
	$(CC) $(ASM_OBJ) $(C_OBJ) -o $(TARGET)

$(ASM_OBJ): $(ASM_SRC)
	$(AS) $(ASFLAGS) $(ASM_SRC) -o $(ASM_OBJ)

$(C_OBJ): $(C_SRC)
	$(CC) $(CFLAGS) -c $(C_SRC) -o $(C_OBJ)

clean:
	rm -f $(ASM_OBJ) $(C_OBJ) $(TARGET)

# 빠른 테스트 (10M iterations)
test-fast: $(TARGET)
	./$(TARGET)

# 중간 테스트 (50M iterations)
test-medium:
	python3 asm_benchmark_generator.py 50000000
	make clean && make
	./$(TARGET)

# 긴 테스트 (200M iterations, 1초+ 보장)
test-long:
	python3 asm_benchmark_generator.py 200000000
	make clean && make
	./$(TARGET)

# 매우 긴 테스트 (500M iterations)
test-very-long:
	python3 asm_benchmark_generator.py 500000000
	make clean && make
	./$(TARGET)

# 성능 카운터와 함께 실행 (perf 필요)
perf-test: $(TARGET)
	perf stat -e cycles,instructions,cache-misses,cache-references,power/energy-pkg/,power/energy-cores/ ./$(TARGET) 2>/dev/null || perf stat -e cycles,instructions,cache-misses,cache-references ./$(TARGET)

# turbostat으로 상세 전력 분석
power-test: $(TARGET)
	@echo "전력 모니터링과 함께 벤치마크 실행 (sudo 필요)..."
	sudo turbostat --show Core,CPU,Avg_MHz,Bzy_MHz,TSC_MHz,PkgWatt,CoreWatt,PkgTmp,CoreTmp --quiet ./$(TARGET)

# powertop으로 시스템 전력 분석
powertop-analysis:
	@echo "시스템 전체 전력 분석 (60초, sudo 필요)..."
	sudo powertop --time=60

# CPU 온도와 주파수 모니터링
monitor-cpu:
	@echo "CPU 상태 실시간 모니터링 (Ctrl+C로 중지)..."
	watch -n 1 "echo '=== CPU 온도/주파수 ===' && sensors | grep -E 'Core|Tdie|Tccd' && echo -e '\\n=== CPU 거버너 ===' && cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor && echo -e '\\n=== 현재 주파수 ===' && cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq | head -6"

# 전력 측정 도구 설치 확인
check-power-tools:
	@echo "=== 전력 모니터링 도구 확인 ==="
	@command -v turbostat >/dev/null 2>&1 && echo "✓ turbostat 설치됨" || echo "✗ turbostat 없음 (sudo apt install turbostat)"
	@command -v powertop >/dev/null 2>&1 && echo "✓ powertop 설치됨" || echo "✗ powertop 없음 (sudo apt install powertop)"
	@command -v sensors >/dev/null 2>&1 && echo "✓ sensors 설치됨" || echo "✗ sensors 없음 (sudo apt install lm-sensors)"
	@ls /sys/class/powercap/intel-rapl* >/dev/null 2>&1 && echo "✓ RAPL 지원됨" || echo "✗ RAPL 지원 안됨"

help:
	@echo "사용 가능한 타겟:"
	@echo ""
	@echo "=== 기본 벤치마크 ==="
	@echo "  make test-fast      # 10M iterations (~0.1-0.5초)"
	@echo "  make test-medium    # 50M iterations (~0.5-2초)"
	@echo "  make test-long      # 200M iterations (~1-5초)"
	@echo "  make test-very-long # 500M iterations (~3-10초)"
	@echo ""
	@echo "=== 전력 모니터링 ==="
	@echo "  make power-test     # turbostat과 함께 실행 (sudo 필요)"
	@echo "  make perf-test      # perf 성능 카운터와 함께"
	@echo "  make powertop-analysis # 시스템 전체 전력 분석"
	@echo "  make monitor-cpu    # CPU 상태 실시간 모니터링"
	@echo "  make check-power-tools # 전력 도구 설치 상태 확인"
	@echo ""
	@echo "=== 수동 설정 ==="
	@echo "  python3 asm_benchmark_generator.py [ITERATIONS]"
	@echo "  make clean && make && ./benchmark"
	@echo ""
	@echo "=== 예시 ==="
	@echo "  make check-power-tools  # 먼저 도구 확인"
	@echo "  make power-test         # 전력과 함께 측정"

.PHONY: all clean test-fast test-medium test-long test-very-long perf-test power-test powertop-analysis monitor-cpu check-power-tools help
'''
        return makefile

    def generate_benchmark(self):
        """전체 벤치마크 시스템 생성"""
        print("Generating assembly instruction benchmark...")

        # 랜덤 데이터 생성
        self.generate_random_data()

        # 어셈블리 파일 생성
        print("Creating assembly file...")
        asm_content = self.generate_assembly_file()
        with open("benchmark.s", "w") as f:
            f.write(asm_content)

        # C 드라이버 생성
        print("Creating C driver...")
        c_content = self.create_c_driver()
        with open("main.c", "w") as f:
            f.write(c_content)

        # Makefile 생성
        print("Creating Makefile...")
        makefile_content = self.create_makefile()
        with open("Makefile", "w") as f:
            f.write(makefile_content)

        print("Benchmark generation complete!")
        print("")
        print("=== Ryzen 5 5600 + 전력 모니터링 사용법 ===")
        print("1. 전력 도구 확인:")
        print("   make check-power-tools")
        print("")
        print("2. 기본 성능 테스트:")
        print("   make test-fast")
        print("")
        print("3. 전력과 함께 측정:")
        print("   make power-test        # turbostat 필요 (sudo)")
        print("")
        print("4. 상세 분석:")
        print("   make perf-test         # 성능 카운터")
        print("   make monitor-cpu       # 실시간 모니터링")
        print("   make powertop-analysis # 시스템 전력 분석")
        print("")
        print("5. 도움말:")
        print("   make help")


if __name__ == "__main__":
    import sys

    # Ryzen 5 5600에 최적화된 설정
    DATA_SIZE = 100000  # L3 캐시(32MB)보다 큰 데이터셋

    # 명령행 인수로 iteration 수 조정 가능
    if len(sys.argv) > 1:
        ITERATIONS = int(sys.argv[1])
        print(f"Using {ITERATIONS:,} iterations from command line")
    else:
        # 기본값: 빠른 테스트로 시작
        ITERATIONS = 10000000  # 10M iterations (약 0.1-0.5초 예상)
        print(f"Using default {ITERATIONS:,} iterations")
        print("실행 후 시간이 너무 짧으면 다음과 같이 늘려보세요:")
        print("  python3 asm_benchmark_generator.py 50000000   # 50M")
        print("  python3 asm_benchmark_generator.py 100000000  # 100M")
        print("  python3 asm_benchmark_generator.py 200000000  # 200M")

    generator = AssemblyBenchmarkGenerator(DATA_SIZE, ITERATIONS)
    generator.generate_benchmark()