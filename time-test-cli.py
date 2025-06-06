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
    return result
print("python code time test program")
code_number =input("the number of code: ")
try:
    test_number = int(code_number)
except ValueError:
    print("error!")
    exit()
code_lists = list()
test_list = list()
for i in range(test_number):
    input_test_name = input("input code name: ")
    test_list.append(input_test_name)
    print(f"code{i+1}:")
    code_lists.append(input())

nRange = int(input("range: "))
result = list()
for code in code_lists:
    test_result = test(code, nRange)
    result.append(test_result)
isSaveResult = input("do you want to save the result? (y/n)")
if isSaveResult == "y":
    save_location = input("input save location: ")
    save_benchmark_to_csv(test_list, result, save_location)
    print("saved successfully!")
else:
    for i in result:
        for j in i:
            meta_print(j)