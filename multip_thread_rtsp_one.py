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

    resize_width = 0
    resize_height = 0

    # c_url = create_string_buffer(url.encode('utf-8'), len(url))
    # ret = rtspclient.createRtspClient(index,url)
    
    while(1) :
        ret1 = rtspclient.isConnect(index)
        print("id %d ret = %d",index, ret1)
        time.sleep(0.1)
        if (ret1 == 1):
            status, img, img_resize = rtspclient.mread(index,width,height,resize_width,resize_height)
            if status == 1:
                # print(type(ret2.frame))
                # print(ret2.frame.shape)
                # print(type(img))
                # img = cv2.resize(img,(1920,1080,3))
                # img = cv2.UMat(height,width,cv2.CV_8UC3,img)
                # img = cv2.UMat(height,width,cv2.CV_8UC3,ret2.frame)
                # cv2.imshow("name", ret2.frame)
                # cv2.waitKey(0)
                pass
                # time.sleep(0.1)
                # cv2.imwrite('a' + str(index) +'.jpg',ret2.frame)
                # cv2.imwrite('a640' + str(index) +'.jpg',ret2.frame_resize)
            else:
                print("python disconnect")
        elif (ret1 == 2):
            print("destoryRtspClient")
            rtspclient.destoryRtspClient(index)
            rtspclient.createRtspClient(index,url)

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

    # t0 = threading.Thread(target=func_rtspdisplay, args = (1,'rtsp://admin:admin@192.168.2.50/cam/realmonitor?channel=1&subtype=0', "admin", "shangqu2020"))
    # t1 = threading.Thread(target=func_rtspdisplay, args = (2,"rtsp://admin:admin@192.168.2.50/cam/realmonitor?channel=1&subtype=0", "admin", "shangqu2020"))
    # t2 = threading.Thread(target=func_rtspdisplay, args = (3,"rtsp://admin:shangqu2020@192.168.2.30/cam/realmonitor?channel=1&subtype=0", "admin", "shangqu2020"))
    # t3 = threading.Thread(target=func_rtspdisplay, args = (4,'rtsp://admin:shangqu2020@192.168.2.30/cam/realmonitor?channel=1&subtype=0', "admin", "shangqu2020"))
    # t4 = threading.Thread(target=func_rtspdisplay, args = (5, "rtsp://admin:shangqu2020@192.168.2.64/Streaming/Channels/1", "admin", "shangqu2020"))
    t5 = threading.Thread(target=func_rtspdisplay, args = (6, "rtsp://admin:shangqu2020@192.168.2.33/Streaming/Channels/1", "admin", "shangqu2020"))

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
