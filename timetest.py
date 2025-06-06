import datetime
import math
import csv

def const(n: int = 1, code: str = "k=n+1"):
    exec(code)

def lg(n, base: int = 2, code: str = "k=n+1"):
    i = 1
    while i < n:
        i *= base
        exec(code)

def linear(n, code: str = "k=n+1"):
    for i in range(n):
        exec(code)

def factorial(n: int, code: str = "k=n+1"):
    for i in range(math.factorial(n)):
        exec(code)

def square(n: int, exp: int = 2, code: str = "k=n+1"):
    for i in range(n**exp):
        exec(code)

def exponential(n: int, base: int = 2, code: str = "k=n+1"):
    for i in range(base**n):
        exec(code)

def meta_print(data: float):
    print(format(data, ".100f"))

def time_test(code: str, n: int)->float:
    start = datetime.datetime.now()
    n=n
    test_list = [i for i in range(n)]
    exec(code, globals(), locals())
    end = datetime.datetime.now()
    result = end-start
    return result.total_seconds()


def save_benchmark_to_csv(test_list, result, filename='benchmark_results.csv'):
    """
    벤치마크 테스트 결과를 CSV 파일로 저장합니다.

    Args:
        test_list: 테스트 요소들의 이름이 담긴 리스트
        result: 각 테스트의 실행 결과가 담긴 2차원 리스트
        filename: 저장할 CSV 파일명
    """

    # 실행 횟수 (n) 계산
    n = len(result[0]) if result else 0

    with open(filename, 'w', newline='', encoding='utf-8') as csvfile:
        writer = csv.writer(csvfile)

        # 헤더 작성 (n, test_list[0], test_list[1], ...)
        header = ['n'] + test_list
        writer.writerow(header)

        # 각 실행 결과를 행으로 작성
        for i in range(n):
            row = [i + 1]  # 실행 번호 (1부터 시작)
            for j in range(len(test_list)):
                row.append(result[j][i])
            writer.writerow(row)

def multi(n: int, code: str = "k=n+1"):
    for i in range(n):
        exec(code)