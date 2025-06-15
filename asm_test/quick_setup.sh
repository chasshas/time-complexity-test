#!/bin/bash

echo "Setting up Assembly Performance Tester..."

# 필요한 패키지 확인 및 설치
echo "Checking required packages..."
sudo apt update
sudo apt install -y build-essential linux-tools-generic

# MSR 모듈 로드
echo "Loading MSR module..."
sudo modprobe msr

# 파일들이 있는지 확인
if [ ! -f "asm_perf_test.c" ]; then
    echo "Error: asm_perf_test.c not found!"
    echo "Please create the C file first."
    exit 1
fi

if [ ! -f "Makefile" ]; then
    echo "Error: Makefile not found!"
    echo "Please create the Makefile first."
    exit 1
fi

echo "Setting up and running initial test..."
make setup
make all

echo ""
echo "Setup complete! You can now run:"
echo "  sudo ./asm_perf_test"
echo "  or"
echo "  ./run_tests.sh"
