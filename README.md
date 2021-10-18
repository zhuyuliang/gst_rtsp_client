# gstreamer_rtsp_client

## gst rtsp _ rk3399prod
### rtspsrc->decodebin(mppvideodec)->tee->queue->appsink->rga

#### 优点:
* 1.多线程rtsp调用
* 2.support h265 h264
* 3.支持分辨率 
    * 640 * 480
    * 1280 * 720
    * 1920 * 1080
    * 2560 * 1440
    * 3840 * 2160

#### rockchip平台依赖环境
* 1.[MPP](https://t.rock-chips.com/forum.php?mod=viewthread&tid=336&highlight=mpp)
* 2.[RGA](https://t.rock-chips.com/forum.php?mod=viewthread&tid=333&highlight=rga)
* 2.[MPP_GSTREAMER插件](https://github.com/rockchip-linux/gstreamer-rockchip)

## gst rtsp _ (deepstream)/tensorrt .. wait ..
### rtspsrc decodebin(Gst-nvvideo4linux2) tee queue Gst-nvvideoconvert appsink (GST-nvinfer Gst-nvtracker)
