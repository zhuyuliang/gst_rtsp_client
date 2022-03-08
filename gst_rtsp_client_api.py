from ctypes import *
import numpy as np
import time

client = cdll.LoadLibrary("build/libRtspClientLib.so")

def createRtspClient( id, url):
    time.sleep(2)
    print("createRtspClient id = %d %s",id, url)
    isSuccess = client.createRtspClient( id,url.encode())
    time.sleep(2)
    return isSuccess

def destoryRtspClientAll():
    time.sleep(2)
    isSuccess = client.destoryRtspClientAll()
    time.sleep(2)
    return isSuccess

def destoryRtspClient(id):
    time.sleep(2)
    isSuccess = client.destoryRtspClient(id)
    time.sleep(2)
    return isSuccess
    
def isConnect( id):
    return client.isConnect(id)
    
# resize_width , resize_height = 640, 640
def mread(id, width, height, resize_width, resize_height):
    urllen = width * height * 3
    c_pbuf = create_string_buffer(''.encode('utf-8'),urllen)
    urllen_resize, c_pbuf_resize = None, None
    if resize_width > 0 and resize_height > 0:
        urllen_resize = resize_width * resize_height * 3
        c_pbuf_resize = create_string_buffer(''.encode('utf-8'),urllen_resize)
    ret = client.mread(id,width,height,resize_width,resize_height,c_pbuf,urllen, c_pbuf_resize, urllen_resize)
    img, img_resize = None, None
    if ret == 1:
        img = np.frombuffer(string_at(c_pbuf,urllen), dtype=np.uint8).reshape(height,width,3)
        if resize_width > 0 and resize_height > 0: 
            img_resize = np.frombuffer(string_at(c_pbuf_resize,urllen_resize), dtype=np.uint8).reshape(resize_width,resize_height,3)
    del urllen, c_pbuf
    del urllen_resize, c_pbuf_resize
    c_pbuf = None
    c_pbuf_resize = None
    del id, width, height, resize_width, resize_height
    return ret, img, img_resize
