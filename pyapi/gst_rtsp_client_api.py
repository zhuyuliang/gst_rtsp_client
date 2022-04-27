from ctypes import *
import numpy as np
import time

# 加载库文件
rtsp_client = cdll.LoadLibrary("build/libRtspClientLib.so")

DEFAULT_CONN_MODE = 0
TCP_CONN_MODE = 1
UDP_CONN_MODE = 2

# 创建RTSP实例
def createRtspClient( id, url, mode = TCP_CONN_MODE):
    print("createRtspClient id = %d %s",id, url)
    isSuccess = rtsp_client.createRtspClient( id,url.encode(),mode)
    return isSuccess

# 销毁全部RTSP连接
def destoryRtspClientAll():
    isSuccess = rtsp_client.destoryRtspClientAll()
    return isSuccess

# 销毁指定ID的RTSP连接
def destoryRtspClient( id):
    isSuccess = rtsp_client.destoryRtspClient(id)
    return isSuccess

# 是否连接
# STATUS_INIT 0 初始化状态
# STATUS_CONNECTED 已连接
# STATUS_DISCONNECT 已断开
# STATUS_CONNECTING 连接中
def isConnect( id):
    return rtsp_client.isConnect(id)
    
# 同步读取帧数据
# return 状态：ret， 设定的原图：img， 裁剪图片： img_resize
def mread( id, width, height, resize_width, resize_height):
    origin_img_size = width * height * 3
    c_pbuf = create_string_buffer(''.encode('utf-8'), origin_img_size)
    resize_img_size, c_pbuf_resize = None, None
    if resize_width > 0 and resize_height > 0:
        resize_img_size = resize_width * resize_height * 3
        c_pbuf_resize = create_string_buffer(''.encode('utf-8'),resize_img_size)
    ret = rtsp_client.mread(id, width, height, resize_width, resize_height, c_pbuf, origin_img_size, c_pbuf_resize, resize_img_size)
    img_origin, img_resize = None, None
    if ret == 1:
        img_origin = np.frombuffer(string_at(c_pbuf, origin_img_size), dtype=np.uint8).reshape(height, width, 3)
        if resize_width > 0 and resize_height > 0: 
            img_resize = np.frombuffer(string_at(c_pbuf_resize, resize_img_size), dtype=np.uint8).reshape(resize_width, resize_height, 3)
    del origin_img_size, c_pbuf
    del resize_img_size, c_pbuf_resize
    return ret, img_origin, img_resize
