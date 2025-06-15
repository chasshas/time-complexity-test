CC = gcc
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
	watch -n 1 "echo '=== CPU 온도/주파수 ===' && sensors | grep -E 'Core|Tdie|Tccd' && echo -e '\n=== CPU 거버너 ===' && cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor && echo -e '\n=== 현재 주파수 ===' && cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq | head -6"

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
