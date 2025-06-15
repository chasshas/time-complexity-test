cat << 'EOF' > run_full_benchmark.sh
#!/bin/bash

echo "🚀 Ryzen 5 5600 Assembly Benchmark - 전체 프로세스"
echo "=================================================="


echo "1️⃣ 시스템 최적화 중..."
sudo systemctl stop xrdp xrdp-sesman 2>/dev/null || true
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor > /dev/null
sudo sync && sudo sysctl vm.drop_caches=3

# 2. 현재 상태 확인
echo "2️⃣ 시스템 상태 확인..."
echo "   메모리: $(free -h | grep Mem | awk '{print $3"/"$2}')"
echo "   CPU 거버너: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)"
echo "   온도: $(sensors 2>/dev/null | grep -E 'Tdie|Core 0' | head -1 | grep -o '+[0-9.]*°C' || echo 'N/A')"

# 3. 벤치마크 생성 (100M iterations)
echo "3️⃣ 벤치마크 생성 중 (100M iterations)..."
python3 asm_test_maker.py 100000000

# 4. 빌드
echo "4️⃣ 빌드 중..."
make clean && make

# 5. 실행
echo "5️⃣ 벤치마크 실행 중..."
echo "   기본 측정..."
./benchmark > results_basic.txt

echo "   전력 분석..."
sudo turbostat --show Core,CPU,Avg_MHz,Bzy_MHz,PkgWatt,PkgTmp --quiet ./benchmark > results_power.txt 2>&1

echo "   성능 카운터..."
perf stat -e cycles,instructions,cache-misses ./benchmark > results_perf.txt 2>&1

# 6. 결과 정리
echo "6️⃣ 결과 정리 중..."
./collect_results.sh

echo "✅ 완료! 결과는 full_results.txt에 저장되었습니다."
echo ""
echo "📊 빠른 요약:"
cat full_results.txt | grep -A 20 "기본 벤치마크 결과" | head -25
EOF

chmod +x run_full_benchmark.sh