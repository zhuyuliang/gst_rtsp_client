from ctypes import *
import numpy as np
import time
import cv2

# 加载库文件
rtsp_client = cdll.LoadLibrary("build/libRtspClientLib.so")

DEFAULT_CONN_MODE = 0
TCP_CONN_MODE = 1
UDP_CONN_MODE = 2

# 创建RTSP实例
def createRtspClient( id, url, mode = TCP_CONN_MODE):
    print("python createRtspClient id = %d %s",id, url)
    isSuccess = rtsp_client.createRtspClient( id,url.encode(),mode)
    return isSuccess

# 销毁全部RTSP连接
def destoryRtspClientAll():
    isSuccess = rtsp_client.destoryRtspClientAll()
    return isSuccess

# # 销毁指定ID的RTSP连接
# def destoryRtspClient( id):
#     isSuccess = rtsp_client.destoryRtspClient(id)
#     return isSuccess

# 是否连接
# STATUS_INIT 0 初始化状态
# STATUS_CONNECTED 已连接
# STATUS_DISCONNECT 已断开
# STATUS_CONNECTING 连接中
def isConnect( id):
    return rtsp_client.isConnect(id)

def changeURL( id, url, mode = TCP_CONN_MODE):
    print("python changeURL id = %d %s",id, url)
    isSuccess = rtsp_client.changeURL( id,url.encode(),mode)
    return isSuccess

def reConnect( id):
    print("python reConnect id = %d %s",id)
    isSuccess = rtsp_client.reConnect( id)
    return isSuccess

# 同步读取帧数据
# return 状态：ret， 设定的原图：img umat
def mread_opencv(id):
    source_width = c_int(0)
    source_height = c_int(0)
    source_size = c_int(0)
    c_pbuf = create_string_buffer(''.encode('utf-8'), 99999999)
    pBuf = c_char_p(addressof(c_pbuf))
    ret = rtsp_client.mRead_Python(id, byref(source_width), 
                            byref(source_height), byref(source_size), pointer(c_pbuf))
    img_resize, img_size = None, None
    if ret == 1:
        print(source_size)
        img_origin = np.frombuffer(string_at(c_pbuf, source_size), dtype=np.uint8).reshape(source_height.value, source_width.value,3)
        bgr_img = cv2.UMat(img_origin)
        img_size = [source_width.value, source_height.value]
        img_resize = bgr_img
    return ret, img_resize, img_size
