#!/usr/bin/env python3.6
import os
import time
import threading
import cv2
import numpy as np
import pyapi.gst_rtsp_client_api as rtspclient

from ctypes import *

# def callback_frame(a,b):
#     print("callback")
#     pass

# functype = CFUNCTYPE(c_int,c_int)
# c_calback_python = functype(callback_frame)
# client.register_py_to_c(c_calback_python)

def func_rtspdisplay(index,url, usr, pwd):

    isConnected = 0
    
    reconnect_time = 2

    width = 1920
    height = 1080

    resize_width = 0
    resize_height = 0

    rtspclient.createRtspClient(index,url)

    while(1) :
        ret1 = rtspclient.isConnect(index)
        print("id %d ret = %d isconnected %b time = %d",index, ret1, isConnected, reconnect_time)
        # time.sleep(0.1)
        if (ret1 == 1):
            print("id %d mread",index)
            status, img, img_resize = rtspclient.mread(index,width,height,resize_width,resize_height)
            if status == 1:
                print("success %d, %d",index,ret1)
                time.sleep(1)
                if (isConnected != 1):
                    isConnected = 1
                if (reconnect_time != 2):
                    reconnect_time = 2
                # pass
                # img = cv2.resize(img1,(1920,1080,3))
                # img = cv2.UMat(height,width,cv2.CV_8UC3,img)
                # img = cv2.UMat(height,width,cv2.CV_8UC3,img1)
                # cv2.imshow("name", ret2.frame)
                # cv2.waitKey(0)
                # time.sleep(0.1)
                cv2.imwrite('a' + str(index) +'.jpg',img)
                # cv2.imwrite('a640' + str(index) +'.jpg',img_resize)
            # elif status == 2:
            #    time.sleep(reconnect_time)
            #    reconnect_time = (reconnect_time - 1) + (reconnect_time -2)
            #    print("python disconnect", index)
            #    rtspclient.createRtspClient(index,url)
            del status, img, img_resize
        elif (isConnected == 1 and ret1 == 2):
            print("timesleep")
            time.sleep(reconnect_time)
            reconnect_time = (reconnect_time - 1) + (reconnect_time -2)
            print("python disconnect", index)
            rtspclient.createRtspClient(index,url)
        else:
            time.sleep(3)
            print("status %d, %d",index,ret1)

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

    # t0 = threading.Thread(target=func_rtspdisplay, args = (1,"rtsp://admin:admin@192.168.2.240/live/thirdstream", "admin", "passwd"))
    # t1 = threading.Thread(target=func_rtspdisplay, args = (2,"rtsp://admin:pwd@192.168.2.27:554/cam/realmonitor?channel=1&subtype=0", "admin", "passwd"))
    # t2 = threading.Thread(target=func_rtspdisplay, args = (3,"rtsp://admin:pwd@192.168.2.141:554/Streaming/Channels/1", "admin", "passwd"))
    # t3 = threading.Thread(target=func_rtspdisplay, args = (4,'rtsp://admin:pwd@192.168.2.141:554/Streaming/Channels/1', "admin", "admin"))
    t0 = threading.Thread(target=func_rtspdisplay, args = (5,"rtsp://admin:admin@192.168.2.33:554/Streaming/Channels/1", "admin", "passwd"))
    # t5 = threading.Thread(target=func_rtspdisplay, args = (6, "rtsp://admin:pwd@192.168.2.30/Streaming/Channels/1", "admin", "passwd"))

    t0.start()
    # t1.start()
    # t2.start()
    # t3.start()
    # t4.start()
    # t5.start()

    t0.join()
    # t1.join()
    # t2.join()
    # t3.join()
    # t4.join()
    # t5.join()
