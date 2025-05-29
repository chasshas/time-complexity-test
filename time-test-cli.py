from timetest import *

def test(input_code: str, n_range: int):
    result = list()
    for i in range(n_range):
        test_result = time_test(input_code, i)
        result.append(test_result)
    for i in result:
        meta_print(i)

print("python code time test program")
print("code:")
code = input()
nRange = input("range: ")
try:
    test(code, int(nRange))
except ValueError:
    print("error!")