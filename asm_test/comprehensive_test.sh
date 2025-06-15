#!/bin/bash
# comprehensive_test.sh - 종합 어셈블리 성능 측정 스크립트

echo "Comprehensive Assembly Performance Testing for AMD Ryzen 5 5600"
echo "================================================================"

# 사전 확인
echo "1. Checking system requirements..."

# CPU 기능 확인
echo "CPU Features:"
cat /proc/cpuinfo | grep flags | head -1 | grep -o -E "(sse2|avx2|popcnt|lzcnt)" | sort -u
echo ""

# 시스템 최적화
echo "2. Optimizing system for accurate measurement..."

# 백그라운드 프로세스 최소화
echo "Stopping unnecessary services..."
sudo systemctl stop bluetooth 2>/dev/null || true
sudo systemctl stop cups 2>/dev/null || true

# CPU 주파수 고정
echo "Setting CPU to performance mode..."
sudo cpupower frequency-set --governor performance 2>/dev/null || echo "Warning: Could not set CPU governor"

# 고우선순위로 실행
echo "3. Running tests with high priority..."
sudo nice -n -20 bash << 'EOF'

# 메모리 정리
echo "Cleaning system memory..."
sync
echo 3 > /proc/sys/vm/drop_caches

# 여러 번 실행하여 일관성 확인
echo "4. Running multiple test iterations..."

for i in {1..3}; do
    echo "=== Test Run $i/3 ==="
    ./asm_perf_test > "comprehensive_result_$i.txt"
    echo "Completed run $i"
    sleep 5  # 시스템이 안정화될 시간
done

echo "5. Analyzing results..."

# Python 분석 스크립트
python3 << 'PYTHON_SCRIPT'
import sys
import glob
import re

def parse_result_file(filename):
    results = {}
    with open(filename, 'r') as f:
        content = f.read()
    
    # 메인 결과 테이블 파싱
    lines = content.split('\n')
    parsing_main = False
    parsing_cache = False
    parsing_category = False
    
    cache_results = {}
    category_results = {}
    
    for line in lines:
        # 메인 테이블 파싱
        if "Instruction" in line and "Cycles" in line and "Time(ns)" in line:
            parsing_main = True
            continue
        elif parsing_main and "----" in line:
            continue
        elif parsing_main and "Cache Hierarchy Analysis:" in line:
            parsing_main = False
            parsing_cache = True
            continue
        elif parsing_cache and "Instruction Categories Performance:" in line:
            parsing_cache = False
            parsing_category = True
            continue
        elif parsing_main and line.strip():
            parts = line.split()
            if len(parts) >= 4 and not line.startswith("Notes"):
                try:
                    instruction = parts[0]
                    cycles = float(parts[1])
                    time_ns = float(parts[2])
                    energy = float(parts[3]) if parts[3] != '0.000000' else 0
                    results[instruction] = {
                        'cycles': cycles,
                        'time': time_ns,
                        'energy': energy
                    }
                except ValueError:
                    continue
        elif parsing_cache and ":" in line:
            # 캐시 결과 파싱
            if "Cache Access:" in line or "RAM Access:" in line:
                parts = line.split(":")
                if len(parts) == 2:
                    cache_type = parts[0].strip()
                    cycles_str = parts[1].strip().replace("cycles", "").strip()
                    try:
                        cache_results[cache_type] = float(cycles_str)
                    except ValueError:
                        continue
        elif parsing_category and "Avg:" in line:
            # 카테고리 결과 파싱
            parts = line.split(":")
            if len(parts) == 2:
                category = parts[0].strip()
                cycles_str = parts[1].strip().replace("cycles", "").strip()
                try:
                    category_results[category] = float(cycles_str)
                except ValueError:
                    continue
    
    return results, cache_results, category_results

# 모든 결과 파일 분석
files = glob.glob("comprehensive_result_*.txt")
if not files:
    print("No result files found!")
    exit(1)

all_results = []
all_cache_results = []
all_category_results = []

for filename in files:
    try:
        results, cache_results, category_results = parse_result_file(filename)
        all_results.append(results)
        all_cache_results.append(cache_results)
        all_category_results.append(category_results)
    except Exception as e:
        print(f"Error parsing {filename}: {e}")

# 평균 계산
def calculate_averages(data_list):
    if not data_list:
        return {}
    
    all_keys = set()
    for data in data_list:
        all_keys.update(data.keys())
    
    averages = {}
    for key in all_keys:
        values = [data[key] for data in data_list if key in data]
        if values:
            if isinstance(values[0], dict):
                # 메인 결과 (딕셔너리)
                avg_cycles = sum(v['cycles'] for v in values) / len(values)
                avg_time = sum(v['time'] for v in values) / len(values)
                avg_energy = sum(v['energy'] for v in values) / len(values)
                
                # 표준편차 계산
                if len(values) > 1:
                    mean_cycles = avg_cycles
                    variance = sum((v['cycles'] - mean_cycles) ** 2 for v in values) / len(values)
                    stddev = variance ** 0.5
                else:
                    stddev = 0
                
                averages[key] = {
                    'cycles': avg_cycles,
                    'time': avg_time,
                    'energy': avg_energy,
                    'stddev': stddev
                }
            else:
                # 캐시/카테고리 결과 (숫자)
                averages[key] = sum(values) / len(values)
    
    return averages

avg_results = calculate_averages(all_results)
avg_cache = calculate_averages(all_cache_results)
avg_category = calculate_averages(all_category_results)

# 결과 출력
print("COMPREHENSIVE PERFORMANCE ANALYSIS")
print("="*60)
print(f"Runs analyzed: {len(files)}")
print()

print("INSTRUCTION PERFORMANCE (Average of {} runs):".format(len(files)))
print("%-25s %10s %10s %10s %8s" % ("Instruction", "Cycles", "Time(ns)", "Energy(J)", "StdDev"))
print("-" * 75)

# 카테고리별로 정렬하여 출력
categories = {
    'Basic Arithmetic': ['ADD', 'SUB', 'IMUL', 'DIV'],
    'Logical Operations': ['AND', 'OR', 'XOR'],
    'Shift Operations': ['SHL', 'SHR'],
    'Basic Instructions': ['MOV', 'CMP'],
    'SSE Instructions': ['SSE2', 'SSE'],
    'AVX Instructions': ['AVX2', 'AVX'],
    'Bit Manipulation': ['POPCNT', 'LZCNT'],
    'Memory Access': ['Memory'],
    'Branch Instructions': ['Branch']
}

for category, keywords in categories.items():
    category_items = []
    for instruction, data in avg_results.items():
        if any(keyword in instruction for keyword in keywords):
            category_items.append((instruction, data))
    
    if category_items:
        print(f"\n{category}:")
        for instruction, data in sorted(category_items):
            print("%-25s %10.3f %10.3f %10.6f %8.3f" % 
                  (instruction, data['cycles'], data['time'], data['energy'], data['stddev']))

print("\nCACHE HIERARCHY PERFORMANCE:")
print("-" * 40)
if avg_cache:
    cache_order = ['L1 Cache Access', 'L2 Cache Access', 'L3 Cache Access', 'RAM Access']
    for cache_type in cache_order:
        if cache_type in avg_cache:
            print(f"{cache_type:20s}: {avg_cache[cache_type]:6.1f} cycles")

print("\nINSTRUCTION CATEGORY AVERAGES:")
print("-" * 40)
if avg_category:
    for category, avg_cycles in sorted(avg_category.items()):
        print(f"{category:25s}: {avg_cycles:6.3f} cycles")

print("\nPERFORMANCE INSIGHTS:")
print("-" * 30)

# 가장 빠른/느린 명령어 찾기
if avg_results:
    fastest = min(avg_results.items(), key=lambda x: x[1]['cycles'])
    slowest = max(avg_results.items(), key=lambda x: x[1]['cycles'])
    
    print(f"Fastest instruction: {fastest[0]} ({fastest[1]['cycles']:.3f} cycles)")
    print(f"Slowest instruction: {slowest[0]} ({slowest[1]['cycles']:.3f} cycles)")
    print(f"Performance ratio: {slowest[1]['cycles']/fastest[1]['cycles']:.1f}x")

# 일관성 분석
inconsistent = [(name, data['stddev']) for name, data in avg_results.items() if data['stddev'] > 0.1]
if inconsistent:
    print(f"\nInconsistent measurements (StdDev > 0.1):")
    for name, stddev in sorted(inconsistent, key=lambda x: x[1], reverse=True)[:5]:
        print(f"  {name}: {stddev:.3f}")

# SIMD 효율성 분석
sse_avg = sum(data['cycles'] for name, data in avg_results.items() if 'SSE' in name) / max(1, len([name for name in avg_results if 'SSE' in name]))
avx_avg = sum(data['cycles'] for name, data in avg_results.items() if 'AVX' in name) / max(1, len([name for name in avg_results if 'AVX' in name]))

if sse_avg > 0 and avx_avg > 0:
    print(f"\nSIMD Analysis:")
    print(f"SSE average: {sse_avg:.3f} cycles")
    print(f"AVX average: {avx_avg:.3f} cycles")
    print(f"AVX efficiency: {sse_avg/avx_avg:.2f}x better than SSE")

print("\nRECOMMendations:")
print("-" * 20)
print("• Use SIMD instructions (SSE/AVX) for parallel operations")
print("• Avoid division when possible - use multiplication by reciprocal")
print("• Memory-bound operations benefit from cache-friendly access patterns")
print("• Consider instruction-level parallelism for better throughput")

PYTHON_SCRIPT

EOF

# 시스템 설정 복구
echo "6. Restoring system settings..."
sudo systemctl start bluetooth 2>/dev/null || true
sudo systemctl start cups 2>/dev/null || true
sudo cpupower frequency-set --governor schedutil 2>/dev/null || true

echo ""
echo "7. Comprehensive testing completed!"
echo "Individual results saved as comprehensive_result_1.txt through comprehensive_result_3.txt"
echo ""
echo "System Analysis Summary:"
echo "========================"

# 시스템 요약 정보
echo "CPU: $(cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d':' -f2 | xargs)"
echo "Cores: $(nproc) cores"
echo "L1d Cache: $(lscpu | grep 'L1d cache' | cut -d':' -f2 | xargs)"
echo "L2 Cache: $(lscpu | grep 'L2 cache' | cut -d':' -f2 | xargs)"
echo "L3 Cache: $(lscpu | grep 'L3 cache' | cut -d':' -f2 | xargs)"
echo "Memory: $(free -h | grep Mem | awk '{print $2}')"

echo ""
echo "For detailed analysis, check the comprehensive results above!"
