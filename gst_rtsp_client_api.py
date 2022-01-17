
from ctypes import *
import numpy as np
import time

'imgframe data class'
class ImgFrameData:
    __slots__ = ['id', 'status', 'width', 'height', 'frame', 'frame_resize']
    def __init__(self, id, status, width, height, frame, frame_resize):
        self.id = id
        self.status = status 
        self.width = width
        self.height = height
        self.frame = frame
        self.frame_resize = frame_resize

client = cdll.LoadLibrary("build/libRtspClientLib.so")

def createRtspClient( id, url):
    time.sleep(1)
    c_url = create_string_buffer(url.encode('utf-8'), len(url))
    return client.createRtspClient( id,c_url,len(url))

def destoryRtspClientAll():
    return client.destoryRtspClientAll()

def destoryRtspClient( id):
    time.sleep(1)
    return client.destoryRtspClient(id)

# def reconnectRtsp( id):
#     return client.reconnectRtsp(id)

def isConnect( id):
    return client.isConnect(id)

# resize_width , resize_height = 640, 640
def mread(id, width, height, resize_width, resize_height):
    urllen = width * height * 3
    c_pbuf = create_string_buffer(''.encode('utf-8'),urllen)
    urllen_resize ,c_pbuf_resize = None, None
    if resize_width > 0 and resize_height > 0:
        urllen_resize = resize_width * resize_height * 3
        c_pbuf_resize = create_string_buffer(''.encode('utf-8'),urllen_resize)
    ret = client.mread(id,width,height,resize_width,resize_height,c_pbuf,urllen, c_pbuf_resize, urllen_resize)
    img, img_resize = None, None
    if ret == 1:
        img = np.frombuffer(string_at(c_pbuf,urllen), dtype=np.uint8).reshape(height,width,3)
        if resize_width > 0 and resize_height > 0: 
            img_resize = np.frombuffer(string_at(c_pbuf_resize,urllen_resize), dtype=np.uint8).reshape(resize_width,resize_height,3)
    return ImgFrameData(id, ret, width, height, img, img_resize)
