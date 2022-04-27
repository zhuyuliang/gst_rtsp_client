#!/usr/bin/env python3.6
from concurrent.futures import thread
from itertools import count
import os
from sys import flags
import time
import threading
import cv2
import numpy as np

import mmap
import contextlib

from multiprocessing import Lock, Process, Condition, Queue

import gst_rtsp_client_api as rtspclient

# from ctypes import *

# if timestamp
# close rm filno
# 

def func_rtspdisplay(index,url, usr, pwd):

    width = 1920
    height = 1080

    resize_width = 0
    resize_height = 0

    # c_url = create_string_buffer(url.encode('utf-8'), len(url))
    # ret = rtspclient.createRtspClient(index,url)
    # write
    filename = './temp/conn'
    if not os.path.exists(filename):
        os.mknod(filename)

    with open(filename, "w") as f:
        f.write('\x00'*7449911)

    with open(filename, 'r+') as f:
        with contextlib.closing(mmap.mmap(f.fileno(),7449911, access=mmap.ACCESS_WRITE)) as m:
            for i in range(1,10001):
                m.seek(0)
                s= "msg" + str(i)
                s.rjust(7449911,'\x00')
                m.write(s.encode())
                time.sleep(1)


    
    # while True:
    #     try:
    #         sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    #         conn = './temp/conn'
    #         sock.connect(conn)
    #     except Exception as e:
    #         print(e)
    #         time.sleep(1)
    #         continue
    #     time.sleep(1)
    #     try:
    #         sock.send('client1'.encode('utf-8'))
    #         sock.setblocking(0) # 0 fei  1 du
    #     except:
    #         continue

    #     while(1) :
    #         ret1 = rtspclient.isConnect(index)
    #         print("id %d ret = %d",index, ret1)
    #         time.sleep(0.1)
    #         if (ret1 == 1):
    #             data = None
    #             try:
    #                 data = sock.recv(1024)
    #                 if len(data) == 0:
    #                     sock.close()
    #                     break
    #                 # print('data:',data.decode('utf-8'))
    #             except BlockingIOError:
    #                 print("BlockingIOError")
    #             except Exception as e:
    #                 print(e)
    #                 print("disconnect")
    #                 sock.close()
    #                 break
                
    #             status, img, img_resize = rtspclient.mread(index,width,height,resize_width,resize_height)
    #             if status == 1:
    #                 # print(type(ret2.frame))
    #                 # print(ret2.frame.shape)
    #                 # print(type(img))
    #                 # img = cv2.resize(img,(1920,1080,3))
    #                 # img = cv2.UMat(height,width,cv2.CV_8UC3,img)
    #                 # img = cv2.UMat(height,width,cv2.CV_8UC3,ret2.frame)
    #                 # cv2.imshow("name", ret2.frame)
    #                 # cv2.waitKey(0)
    #                 if data:
    #                     print("send")
    #                     sock.send('frame1'.encode())
    #                 else:
    #                     print('no send')
                    
    #                 # pass
    #                 # time.sleep(0.1)
    #                 # cv2.imwrite('a' + str(index) +'.jpg',ret2.frame)
    #                 # cv2.imwrite('a640' + str(index) +'.jpg',ret2.frame_resize)
    #             else:
    #                 print("python disconnect")
                
    #         elif (ret1 == 2):
    #             print("destoryRtspClient")
    #             rtspclient.destoryRtspClient(index)
    #             rtspclient.createRtspClient(index,url)

    print('# End of Thread %d' % (index))


if __name__ == '__main__':
    os.system('iptables -F')  # Disable Firewall

    #gl = toy.output.createGLDrmDisplay(toy.DisplayPort.HDMI_A)
    #idx0 = gl.add_view(0, 180, 640, 360)
    # idx1 = gl.add_view(640, 180, 640, 360)
    # idx2 = gl.add_view(1280, 180, 640, 360)
    # idx3 = gl.add_view(0, 540, 640, 360)
    # idx4 = gl.add_view(640, 540, 640, 360)
    # idx5 = gl.add_view(1280, 540, 640, 360)

    t5 = Process(target=func_rtspdisplay, args = (6, "rtsp://admin:passwd@192.168.2.32/Streaming/Channels/1", "admin", "passwd"))

    t5.start()

    t5.join()
