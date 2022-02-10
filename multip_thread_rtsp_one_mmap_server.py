# coding:utf-8
# 服务端
import os
import mmap
import contextlib
import threading
import time

if __name__ == '__main__':

    # read
    filename = './temp/conn'
    if not os.path.exists(filename):
        os.mknod(filename)
    with open(filename, "w") as f:
        f.write('\x00'*7449911)

    while True:
        print("1")
        with open(filename,'r') as f:
            with contextlib.closing(mmap.mmap(f.fileno(),7449911,access=mmap.ACCESS_READ)) as m:
                s = m.read(7449911).decode().replace('\x00','')
                print(s)
                time.sleep(1)
