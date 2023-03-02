#!/usr/bin/env python3.6
import os
import time
import threading
import cv2
import numpy as np
import pyapi.gst_rtsp_client_api as rtspclient
from multiprocessing import Lock, Process, Condition, Queue


# def callback_frame(a,b):
#     print("callback")
#     pass

# functype = CFUNCTYPE(c_int,c_int)
# c_calback_python = functype(callback_frame)
# client.register_py_to_c(c_calback_python)

def func_rtspdisplay(index,url, usr, pwd):

    width = 1920
    height = 1080

    resize_width = 0
    resize_height = 0

    rtspclient.createRtspClient(index,url)

    time.sleep(0.5)

    while(1) :
        ret1 = rtspclient.isConnect(index)
        print("id %d ret = %d",index, ret1)
        time.sleep(0.5)
        if (ret1 == 1):
            # print("id %d mread",index)
            # status, img, img_resize = rtspclient.mread_rga(index,width,height,resize_width,resize_height)
            # if status == 1:
            #     # print("success %d, %d",index,ret1)
            #     # time.sleep(1)
            #     # cv2.imwrite('a' + str(index) +'.jpg',img)
            #     pass
            # else:
            #     print("python mread_rga disconnect")
            status, img, img_size = rtspclient.mread_opencv(index)
            if status == 1:
                img = cv2.UMat.get(img)
                cv2.imwrite('a' + str(index) +'.jpg',img)
            else:
                print("python mread_opencv disconnect")
            del status, img, img_size
        else:
            time.sleep(3)
            rtspclient.reConnect(index)
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

    t0 = threading.Thread(target=func_rtspdisplay, args = (1,"rtsp://admin:psswd@192.168.2.29:554/cam/realmonitor?channel=1&subtype=0", "admin", "psswd"))
    # t1 = threading.Thread(target=func_rtspdisplay, args = (2,"rtsp://admin:psswd@192.168.2.24:554/cam/realmonitor?channel=1&subtype=0", "admin", "psswd"))
    # t2 = threading.Thread(target=func_rtspdisplay, args = (3,"rtsp://admin:psswd@192.168.2.8:554/Streaming/Channels/101", "admin", "psswd"))
    # t3 = threading.Thread(target=func_rtspdisplay, args = (4,'rtsp://admin:psswd@192.168.2.27:554/cam/realmonitor?channel=1&subtype=0', "admin", "psswd"))
    # t4 = threading.Thread(target=func_rtspdisplay, args = (5,"rtsp://admin:psswd@192.168.2.29:554/cam/realmonitor?channel=1&subtype=0", "admin", "psswd"))
    # t5 = threading.Thread(target=func_rtspdisplay, args = (6, "rtsp://admin:psswd@192.168.2.8:554/Streaming/Channels/201", "admin", "psswd"))
    # t6 = threading.Thread(target=func_rtspdisplay, args = (7, "rtsp://admin:psswd@192.168.2.33:554/Streaming/Channels/1", "admin", "psswd"))
    # t7 = threading.Thread(target=func_rtspdisplay, args = (8, "rtsp://admin:psswd@192.168.2.39:554/Streaming/Channels/1", "admin", "psswd"))

    t0.start()
    # t1.start()
    # t2.start()
    # t3.start()
    # t4.start()
    # t5.start()
    # t6.start()
    # t7.start()

    t0.join()
    # t1.join()
    # t2.join()
    # t3.join()
    # t4.join()
    # t5.join()
    # t6.join()
    # t7.join()
