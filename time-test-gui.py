import tkinter.messagebox
import tkinter.filedialog
import tkinter
import csv
from timetest import *

def test(input_code: str, n_range: int):
    test_info = tkinter.Toplevel(window)
    test_info.title("test info")
    test_info.geometry("300x200")
    test_info_msg = tkinter.Label(test_info, text = "test progress: 0/0")
    test_info_msg.pack()
    window.update()
    test_info.update()
    result = list()
    for i in range(n_range):
        test_info_msg.config(text = f"test progress: {i+1}/{n_range}")
        test_info.update()
        test_result = time_test(input_code, i)
        result.append(test_result)
    test_info.destroy()
    is_save_file = tkinter.messagebox.askyesno("finish", "test was done. do you want to save the result?")
    if is_save_file:
        with open(tkinter.filedialog.asksaveasfilename(defaultextension=".csv"), "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["test_number", "test_time"])
        tkinter.messagebox.showinfo("info", "test was saved")
    else:
        tkinter.messagebox.showinfo("info", "test was not saved")
    for i in result:
        meta_print(i)

window = tkinter.Tk()
window.geometry("800x500")
titleFrame = tkinter.Frame(window)
rangeFrame = tkinter.Frame(window)
startBtnFrame = tkinter.Frame(window)
window.title("python code time test program")
titleLbl = tkinter.Label(titleFrame, text = "python code time test program", font=("applegothic", 40))
codeEntry = tkinter.Text(titleFrame, width=100)
rangeEntryIndicatorLbl = tkinter.Label(rangeFrame, text = "range setting")
rangeEntry = tkinter.Entry(rangeFrame)
startBtn = tkinter.Button(startBtnFrame, text="start", command=lambda: test(codeEntry.get("1.0", tkinter.END), int(rangeEntry.get())))
#mainloop
#frames
titleFrame.pack()
rangeFrame.pack()
startBtnFrame.pack()
#title&code input
titleLbl.pack()
codeEntry.pack()
#range input
rangeEntryIndicatorLbl.grid(row = 1,column=2)
rangeEntry.grid(row=1, column=3)
#start button
startBtn.pack()

window.mainloop()