import datetime
import math

def constant(n: int, code: str):
    exec(code)

def logarithmic(n,base: int, code: str):
    i = 1
    while i < n:
        i *= base
        exec(code)

def linear(n, code: str):
    for i in range(n):
        exec(code)

def factorial(n: int, code: str):
    for i in range(math.factorial(n)):
        exec(code)

def square(n: int, exp: int, code: str):
    for i in range(n**exp):
        exec(code)

def exponential(n: int, base: int, code: str):
    for i in range(base**n):
        exec(code)

def meta_print(data: float):
    print(format(data, ".100f"))

def time_test(code: str, n: int)->float:
    start = datetime.datetime.now()
    n=n
    test_list = [i for i in range(n)]
    exec(code)
    end = datetime.datetime.now()
    result = end-start
    return result.total_seconds()
