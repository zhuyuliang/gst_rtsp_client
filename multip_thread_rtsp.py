#!/usr/bin/env python3.6
import os
import time
import threading
import cv2
import numpy as np

from multiprocessing import Lock, Process, Condition, Queue

import gst_rtsp_client_api as rtspclient

# from ctypes import *

def func_rtspdisplay(index,url, usr, pwd):

    width = 1920
    height = 1080

    # c_url = create_string_buffer(url.encode('utf-8'), len(url))
    # ret = rtspclient.createRtspClient(index,url)
    
    while(1) :
        ret1 = rtspclient.isConnect(index)
        time.sleep(0.5)
        print("id %d ret = %d",index, ret1)
        if (ret1 == 1):
            ret2 = rtspclient.mread(index,width,height)
            print(ret2.status)
            # print(type(img))
            # img = cv2.resize(img,(1920,1080,3))
            # img = cv2.UMat(height,width,cv2.CV_8UC3,img)
            cv2.imwrite('a' + str(index) +'.jpg',ret2.frame)
        elif (ret1 == 2):
            rtspclient.destoryRtspClient(index)
            #time.sleep(4)
            rtspclient.createRtspClient(index,url)
            #time.sleep(10)

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

    # t0 = threading.Thread(target=func_rtspdisplay, args = (1,'rtsp://admin:shangqu2020@192.168.2.30/cam/realmonitor?channel=1&subtype=0', "admin", "shangqu2020"))
    # t1 = threading.Thread(target=func_rtspdisplay, args = (2,"rtsp://admin:shangqu2020@192.168.2.29/cam/realmonitor?channel=1&subtype=0", "admin", "shangqu2020"))
    # t2 = threading.Thread(target=func_rtspdisplay, args = (3,"rtsp://admin:shangqu2020@192.168.2.26/cam/realmonitor?channel=1&subtype=0", "admin", "shangqu2020"))
    # t3 = threading.Thread(target=func_rtspdisplay, args = (4,'rtsp://admin:shangqu2020@192.168.2.27/cam/realmonitor?channel=1&subtype=0', "admin", "shangqu2020"))
    # t4 = threading.Thread(target=func_rtspdisplay, args = (5, "rtsp://admin:shangqu2020@192.168.2.32/Streaming/Channels/1", "admin", "shangqu2020"))
    t5 = threading.Thread(target=func_rtspdisplay, args = (6, "rtsp://admin:shangqu2020@192.168.2.64/Streaming/Channels/2", "admin", "shangqu2020"))

    # t0.start()
    # t1.start()
    # t2.start()
    # t3.start()
    # t4.start()
    t5.start()

    # t0.join()
    # t1.join()
    # t2.join()
    # t3.join()
    # t4.join()
    t5.join()