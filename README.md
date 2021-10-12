# gstreamer_rtsp_client

## gst rtsp _ rk3399prod
rtspsrc decodebin(mppvideodec) tee queue appsink rga

question:
1.multi thread rtsp
2.support h265 h264
3.support resolution 1280*720, 1920*1080, 2560*1440

## gst rtsp _ (deepstream)/tensorrt .. wait ..
rtspsrc decodebin(Gst-nvvideo4linux2) tee queue Gst-nvvideoconvert appsink (GST-nvinfer Gst-nvtracker)

question:
1.
