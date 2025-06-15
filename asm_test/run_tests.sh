#!/bin/bash
# run_tests.sh - 종합 테스트 실행 스크립트

set -e

echo "AMD Ryzen 5 5600 Assembly Performance Test Suite"
echo "================================================"

# 권한 확인
if [ "$EUID" -ne 0 ]; then
    echo "Warning: Running without root privileges."
    echo "Energy measurement will not be available."
    echo "Run with sudo for full functionality."
    echo ""
fi

# 시스템 정보 출력
echo "System Information:"
echo "- CPU: $(cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d':' -f2 | xargs)"
echo "- Kernel: $(uname -r)"
echo "- GCC Version: $(gcc --version | head -1)"
echo ""

# MSR 모듈 로드 시도
if [ "$EUID" -eq 0 ]; then
    echo "Loading MSR module..."
    modprobe msr 2>/dev/null || echo "Warning: Could not load MSR module"
    chmod 644 /dev/cpu/*/msr 2>/dev/null || true
fi

# 컴파일
echo "Compiling test program..."
make clean
make all

echo ""
echo "Starting performance tests..."
echo "This may take several minutes..."
echo ""

# CPU 주파수 정보
echo "Current CPU frequency governor: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo 'unknown')"
echo "Current CPU frequency: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq 2>/dev/null || echo 'unknown') kHz"
echo ""

# 테스트 실행
./asm_perf_test

echo ""
echo "Test completed!"
echo ""
echo "Tips for more accurate results:"
echo "1. Run 'sudo cpupower frequency-set --governor performance' to disable frequency scaling"
echo "2. Close other applications to reduce system load"
echo "3. Run multiple times and average the results"
echo "4. Ensure adequate cooling to prevent thermal throttling"
