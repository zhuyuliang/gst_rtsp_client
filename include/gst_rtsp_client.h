//
//  Created by steve 21-09-16
//
//  gstreamer如何接入RTSP流（IP摄像头） 1. h264/h265 2. udp tcp
//  rtspsrc decodebin(h265/264 mppvideodec) tee queue appsink -> rga 

#ifndef SQ_RTSP_H
#define SQ_RTSP_H

#include <functional>
#include <string>

#include <string>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// gst
#include <gst/gst.h>
#include <gst/gstmemory.h>
#include <gst/gstpad.h>
#include <gst/allocators/gstdmabuf.h>
#include <gst/app/gstappsink.h>
#include <gst/gstpipeline.h>
#include <gst/gstcaps.h>

#include <gst/video/gstvideometa.h>
#include <gst/base/gstbasesink.h>
#include <gst/video/video-info.h>
#include <gst/video/video-format.h>
#include <gst/video/video-enumtypes.h>
#include <gst/video/video-tile.h>
#include <gst/base/base-prelude.h>
#include <gst/gstquery.h>

#include <gst/sdp/gstsdpmessage.h>
#include <gobject/gclosure.h>
#include <gst/rtsp/gstrtspmessage.h>

// rga
#include <rga/rga.h>
#include <rga/RgaUtils.h>
#include <rga/RockchipRga.h>
#include <rga/im2d.h>

// opencv2
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

// gst log config
//GST_DEBUG_CATEGORY_STATIC (rk_appsink_debug);
//#define GST_CAT_DEFAULT rk_appsink_debug

#define DISPLAY FALSE

//rga
#define SRC_FORMAT RK_FORMAT_YCrCb_420_SP
#define DST_FORMAT RK_FORMAT_RGB_888

#define STATUS_INIT 0
#define STATUS_CONNECTED 1
#define STATUS_DISCONNECT 2
#define STATUS_CONNECTING 3

inline static const char *
yesno (int yes)
{
  return yes ? "yes" : "no";
}

typedef std::function<void()> FRtspCallBack;

// typedef std::function<void()> REConnect;

struct MppFrameData {
    std::string data;
    int size;
};

struct FrameData {
    char * data;
    int size;
    int width;
    int height;
    int isRun = STATUS_INIT;
    // inference 640 scale
    int size_resize;
    char * data_resize;
};

// CustomData
struct CustomData {
    GMainLoop  *loop;
    GstElement *pipeline;
    GstElement *rtspsrc;
    GstElement *decode;
    GstElement *tee;
    GstElement *queue_appsink;
    GstElement *queue_displaysink;
    GstElement *appsink;
    GstElement *displaysink;

    GstPad *decode_sinkpad;
    GstPad *tee_sinkpad;

    GstBus *bus;
    GstPad *apppad;
    GstPad *queue1_video_pad;
    GstPad *queue2_video_pad;
    GstPad *tee1_video_pad;
    GstPad *tee2_video_pad;

    pthread_t gst_thread;

    gint format;
    GstVideoInfo info;

    unsigned frame;

    char * m_RtspUri;
    int url_size = 0;
    int m_Id = 0; 
    FRtspCallBack m_RtspCallBack;
    int isRun = STATUS_INIT;

    // ArrayQueue<MppFrameData> *mqueue;

    // rga buf
    char * dst_buf;
    char * dst_output_buf;
    char * dst_resize_output_buf;

    // inference resize 640
    char * dst_resize_output_resize_buf;
    char * dst_output_resize_buf;
};

class RtspClient {
public:

    RtspClient();
    ~RtspClient();
    
    bool enable(int id, const char * url, int urllen);
    bool enable(int id, const char * url, int urllen, FRtspCallBack callBack);
    void disable();
    // int reconnect();
    int isConnect();

    struct FrameData * read(int width, int height, int resize_width, int resize_height);

private:
    pthread_t m_thread;
    struct CustomData *m_data = NULL;

};

#endif //SQ_RTSP_H
