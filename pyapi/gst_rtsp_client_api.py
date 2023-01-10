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
# return 状态：ret， 设定的原图：img， 裁剪图片： img_resize
def mread_rga( id, width, height, resize_width, resize_height):
    origin_img_size = width * height * 3
    c_pbuf = create_string_buffer(''.encode('utf-8'), origin_img_size)
    resize_img_size, c_pbuf_resize = None, None
    if resize_width > 0 and resize_height > 0:
        resize_img_size = resize_width * resize_height * 3
        c_pbuf_resize = create_string_buffer(''.encode('utf-8'),resize_img_size)
    ret = rtsp_client.mRead_Rga(id, width, height, resize_width, resize_height, c_pbuf, origin_img_size, c_pbuf_resize, resize_img_size)
    img_origin, img_resize = None, None
    if ret == 1:
        img_origin = np.frombuffer(string_at(c_pbuf, origin_img_size), dtype=np.uint8).reshape(height, width, 3)
        if resize_width > 0 and resize_height > 0: 
            img_resize = np.frombuffer(string_at(c_pbuf_resize, resize_img_size), dtype=np.uint8).reshape(resize_width, resize_height, 3)
    del origin_img_size, c_pbuf
    del resize_img_size, c_pbuf_resize
    return ret, img_origin, img_resize

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
        img_origin = np.frombuffer(string_at(c_pbuf, source_size), dtype=np.uint8).reshape(int(source_height.value * 3 / 2), source_width.value)
        yuvNV12 = cv2.UMat(img_origin)
        # rgb24 = cv2.Mat(source_height.value, source_width.value , cv2.CV_8UC3)
        rgb24 = cv2.cvtColor(yuvNV12 , cv2.COLOR_YUV2BGR_NV12)
        # h265 256bit 1920 * 1080  == 2304 * 1080
        if (source_size.value == 3732480 or source_size.value == 4976640 ):
            img_resize = cv2.UMat(rgb24,[0,1080],[0,1920])
            img_size = [1920, 1080]
        # h264 16bit 1920 * 1080 == 1920 * 1088
        elif (source_size.value == 3133440 or source_size.value == 4177920 ):
            img_resize = cv2.UMat(rgb24,[0,1080],[0,1920])
            img_size = [1920, 1080]
        # h265 2560 * 1440 256 2816 1584
        elif (source_size.value == 6082560 or source_size.value == 8110080):
            img_resize = cv2.UMat(rgb24,[0,1440],[0,2560])
            img_size = [2560, 1440]
        # h265 640*480 768 * 480 
        elif (source_size.value == 552960):
            img_resize = cv2.UMat(rgb24,[0,480],[0,640])
            img_size = [640, 480]
        else:
        # h265 h264
        # supoort 1280*720 3840*2160
            img_resize = rgb24
            img_size = [source_width.value, source_height.value]
    return ret, img_resize, img_size
