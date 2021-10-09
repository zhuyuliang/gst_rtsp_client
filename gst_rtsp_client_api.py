
from ctypes import *
import numpy as np
import time

'imgframe data class'

class ImgFrameData:
    id = 0
    status = 0
    width = 0
    height = 0 
    frame = None

    def __init__(self, id, status, width, height, frame):
        self.id = id
        self.status = status 
        self.width = width
        self.frame = frame
        self.height = height

client = cdll.LoadLibrary("build/libRtspClientLib.so")

def createRtspClient( id, url):
    time.sleep(1)
    c_url = create_string_buffer(url.encode('utf-8'), len(url))
    return client.createRtspClient( id,c_url,len(url))
    # time.sleep(2)
    # return ret

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
    ret = client.mread(id,width,height,c_pbuf,urllen)
    img = np.frombuffer(string_at(c_pbuf,urllen), dtype=np.uint8).reshape(height,width,3)
    return ImgFrameData(id,ret,width,height,img)
