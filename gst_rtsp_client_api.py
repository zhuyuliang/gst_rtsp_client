
from ctypes import *
import numpy as np
import time

'imgframe data class'

class ImgFrameData:
    __slots__ = ['id', 'status', 'width', 'height', 'frame', 'frame640']
    def __init__(self, id, status, width, height, frame, frame640):
        self.id = id
        self.status = status 
        self.width = width
        self.height = height
        self.frame = frame
        self.frame640 = frame640


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

def mread(id, width, height):
    urllen = width * height * 3
    c_pbuf = create_string_buffer(''.encode('utf-8'),urllen)
    urllen640 = 640 * 640 * 3
    c_pbuf640 = create_string_buffer(''.encode('utf-8'),urllen640)
    ret = client.mread(id,width,height,c_pbuf,urllen, c_pbuf640, urllen640)
    img = np.frombuffer(string_at(c_pbuf,urllen), dtype=np.uint8).reshape(height,width,3)
    img640 = np.frombuffer(string_at(c_pbuf640,urllen640), dtype=np.uint8).reshape(640,640,3)
    return ImgFrameData(id,ret,width,height,img,img640)
