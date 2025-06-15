# Comprehensive Assembly Performance Tester Makefile
CC = gcc
CFLAGS = -O1 -march=native -mtune=native -mavx2 -msse4.2 -mpopcnt -mlzcnt -Wall -Wextra -fno-builtin
TARGET = asm_perf_test
SOURCE = comprehensive_asm_test.c
LIBS = -lm
SCRIPT = comprehensive_test.sh

.PHONY: all clean run setup debug asm help fix-freq restore-freq comprehensive quick install-deps

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)

# 종합 테스트 실행 (권장)
comprehensive: $(TARGET) $(SCRIPT)
	@echo "Running comprehensive performance analysis..."
	chmod +x $(SCRIPT)
	./$(SCRIPT)

# 빠른 테스트 (단일 실행)
quick: $(TARGET)
	@echo "Running quick performance test..."
	sudo ./$(TARGET)

# 기본 실행 (이전 버전과 호환)
run: quick

# 시스템 설정
setup:
	@echo "Setting up system for performance testing..."
	sudo modprobe msr
	sudo chmod 644 /dev/cpu/*/msr 2>/dev/null || true
	@echo "Checking CPU features..."
	@cat /proc/cpuinfo | grep flags | head -1 | grep -o -E "(sse2|avx2|popcnt|lzcnt)" | sort -u || echo "Some features may not be available"
	@echo "Setup complete."

# 필요한 패키지 설치
install-deps:
	@echo "Installing required packages..."
	sudo apt update
	sudo apt install -y build-essential linux-tools-generic python3
	@echo "Dependencies installed."

# CPU 주파수 고정 (더 정확한 측정을 위해)
fix-freq:
	@echo "Setting CPU to performance mode..."
	sudo cpupower frequency-set --governor performance
	@echo "CPU frequency scaling disabled."

# CPU 주파수 스케일링 복구
restore-freq:
	@echo "Restoring CPU frequency scaling..."
	sudo cpupower frequency-set --governor schedutil
	@echo "CPU frequency scaling restored."

# 디버그 빌드 (최적화 없음)
debug: $(SOURCE)
	$(CC) -O0 -g -march=native -mavx2 -msse4.2 -mpopcnt -mlzcnt -Wall -Wextra -fno-builtin -o $(TARGET)_debug $(SOURCE) $(LIBS)
	@echo "Debug build created: $(TARGET)_debug"

# 어셈블리 출력 생성
asm: $(SOURCE)
	$(CC) $(CFLAGS) -S -o $(TARGET).s $(SOURCE)
	@echo "Assembly output saved to $(TARGET).s"
	@echo "Use 'less $(TARGET).s' to view the generated assembly code"

# 성능 분석 (perf 도구 사용)
perf-analysis: $(TARGET)
	@echo "Running detailed performance analysis with perf..."
	@echo "This requires root privileges and perf tools"
	sudo perf stat -e cycles,instructions,cache-references,cache-misses,branch-instructions,branch-misses ./$(TARGET)

# 벤치마크 비교
benchmark: $(TARGET)
	@echo "Running benchmark comparison..."
	@echo "Testing with different compiler optimizations..."
	$(CC) -O0 -march=native -o $(TARGET)_O0 $(SOURCE) $(LIBS)
	$(CC) -O2 -march=native -o $(TARGET)_O2 $(SOURCE) $(LIBS)
	$(CC) -O3 -march=native -o $(TARGET)_O3 $(SOURCE) $(LIBS)
	@echo "Running O0 build..."
	sudo ./$(TARGET)_O0 > benchmark_O0.txt
	@echo "Running O2 build..."
	sudo ./$(TARGET)_O2 > benchmark_O2.txt
	@echo "Running O3 build..."
	sudo ./$(TARGET)_O3 > benchmark_O3.txt
	@echo "Benchmark results saved to benchmark_*.txt files"

# 결과 정리
clean-results:
	rm -f result_*.txt comprehensive_result_*.txt benchmark_*.txt

# 정리
clean:
	rm -f $(TARGET) $(TARGET)_debug $(TARGET)_O* $(TARGET).s

# 완전 정리
clean-all: clean clean-results

# 시스템 정보 출력
sysinfo:
	@echo "System Information for Performance Testing:"
	@echo "==========================================="
	@echo "CPU Model: $$(cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d':' -f2 | xargs)"
	@echo "CPU Cores: $$(nproc) cores"
	@echo "CPU Features: $$(cat /proc/cpuinfo | grep flags | head -1 | grep -o -E '(sse|sse2|sse3|ssse3|sse4_1|sse4_2|avx|avx2|fma|popcnt|lzcnt|bmi1|bmi2)' | sort -u | tr '\n' ' ')"
	@echo "L1d Cache: $$(lscpu | grep 'L1d cache' | cut -d':' -f2 | xargs)"
	@echo "L1i Cache: $$(lscpu | grep 'L1i cache' | cut -d':' -f2 | xargs)"
	@echo "L2 Cache: $$(lscpu | grep 'L2 cache' | cut -d':' -f2 | xargs)"
	@echo "L3 Cache: $$(lscpu | grep 'L3 cache' | cut -d':' -f2 | xargs)"
	@echo "Memory: $$(free -h | grep Mem | awk '{print $$2}')"
	@echo "Current CPU Governor: $$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo 'unknown')"
	@echo "GCC Version: $$(gcc --version | head -1)"
	@echo "Kernel: $$(uname -r)"

# 도움말
help:
	@echo "Comprehensive Assembly Performance Tester"
	@echo "========================================"
	@echo ""
	@echo "Available targets:"
	@echo "  comprehensive  - Run full analysis with multiple iterations (RECOMMENDED)"
	@echo "  quick         - Run single performance test"
	@echo "  setup         - Setup MSR access and check system"
	@echo "  install-deps  - Install required system packages"
	@echo "  fix-freq      - Disable CPU frequency scaling"
	@echo "  restore-freq  - Restore CPU frequency scaling"
	@echo "  debug         - Compile with debug symbols and no optimization"
	@echo "  asm           - Generate assembly output"
	@echo "  benchmark     - Compare different compiler optimizations"
	@echo "  perf-analysis - Run detailed perf analysis"
	@echo "  sysinfo       - Display system information"
	@echo "  clean         - Remove compiled files"
	@echo "  clean-results - Remove result files"
	@echo "  clean-all     - Remove all generated files"
	@echo ""
	@echo "Testing Instructions Covered:"
	@echo "  • Basic Arithmetic: ADD, SUB, IMUL, DIV"
	@echo "  • Logical Operations: AND, OR, XOR"
	@echo "  • Shift Operations: SHL, SHR"
	@echo "  • Basic Instructions: MOV, CMP"
	@echo "  • SSE Instructions: PADDQ, ADDPS, MULPS"
	@echo "  • AVX Instructions: VPADDQ, VADDPS, VMULPS"
	@echo "  • Bit Manipulation: POPCNT, LZCNT"
	@echo "  • Memory Hierarchy: L1, L2, L3, RAM access patterns"
	@echo "  • Branch Instructions: Conditional branches"
	@echo ""
	@echo "Quick Start Guide:"
	@echo "  1. make install-deps    # Install required packages"
	@echo "  2. make setup          # Setup system access"
	@echo "  3. make sysinfo        # Check system capabilities"
	@echo "  4. make fix-freq       # Optional: Fix CPU frequency"
	@echo "  5. make comprehensive  # Run full test suite"
	@echo "  6. make restore-freq   # Optional: Restore frequency scaling"
	@echo ""
	@echo "For troubleshooting:"
	@echo "  • make debug && sudo ./asm_perf_test_debug"
	@echo "  • make asm    # Check compiler output"
	@echo "  • make benchmark  # Compare optimization levels"
