import csv
import sys
from timetest import *

def test(input_code: str, n_range: int):
    result = list()
    for i in range(n_range):
        print(f"test progress: {i+1}/{n_range}", end="\r")
        test_result = time_test(input_code, i)
        result.append(test_result)
    for i in result:
        meta_print(i)
    is_save = input("do you want to save the result? (y/n)")
    if is_save == "y":
        save_loc = input("save location: ")
        with open(save_loc, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["test_number", "test_time"])
        print("saved")
    else:
        print("not saved")
print("python code time test program")
code_number =input("the number of code: ")
try:
    test_number = int(code_number)
except ValueError:
    print("error!")
    exit()

test_codes = list()
for i in range(test_number):
    print(f"code{i+1}:")
    code_lines = list()
    while True:
        line = sys.stdin.readline().rstrip('\n')
        if not line:  # 빈 줄이면 종료
            break
        code_lines.append(line)
    test_codes.append(code_lines)

code_lists = list()

for i in test_codes:
    code = '\n'.join(i)
    code_lists.append(code)


nRange = input("range: ")
try:
    for code in code_lists:
        test(code, int(nRange))
except ValueError:
    print("error!")
print()