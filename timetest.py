import datetime
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