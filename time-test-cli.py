import csv

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
print("code:")
code = input()
nRange = input("range: ")
try:
    test(code, int(nRange))
except ValueError:
    print("error!")