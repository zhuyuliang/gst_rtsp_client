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

// opencv2
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

// gst log config
//GST_DEBUG_CATEGORY_STATIC (rk_appsink_debug);
//#define GST_CAT_DEFAULT rk_appsink_debug

#define STATUS_INIT 0
#define STATUS_CONNECTED 1
#define STATUS_DISCONNECT 2
#define STATUS_CONNECTING 3

#define DEFAULT_CONN_MODE 0
#define TCP_CONN_MODE 1
#define UDP_CONN_MODE 2

#define MUXER_OUTPUT_WIDTH 1920
#define MUXER_OUTPUT_HEIGHT 1080
#define MUXER_BATCH_TIMEOUT_USEC 4000000

inline static const char *
yesno (int yes)
{
  return yes ? "yes" : "no";
}

struct MppFrameData {
    std::string data;
    int size;
};

struct FrameData {
    char * data = NULL;
    int size;
    int width;
    int height;
    int isRun = STATUS_INIT;
    // inference scale
    int size_resize;
    char * data_resize = NULL;
};

// CustomData
struct CustomData {
    GMainLoop  *loop = NULL;
    GstElement *pipeline = NULL;
    GstElement *source_bin = NULL;
    GstElement *uridecodebin = NULL;
    GstElement *nvvideoconvert = NULL;
    GstElement *capsfilter = NULL;
    GstElement *appsink = NULL;

    GstBus *bus = NULL;

    gint format;
    GstVideoInfo info;
    int last_time_width = 0;
    int last_time_hetight = 0;

    unsigned frame = 0;

    char * m_RtspUri = NULL;
    int m_Id = 0;
    int conn_Mode = DEFAULT_CONN_MODE; 
    int isRun = STATUS_INIT;

    // rga buf
    char * dst_buf = NULL;
    char * dst_output_buf = NULL;
    char * dst_resize_output_buf = NULL;

    // inference resize 640
    char * dst_resize_output_resize_buf = NULL;
    char * dst_output_resize_buf = NULL;
};

class RtspClient {
public:

    RtspClient();
    ~RtspClient();
    
    bool enable(int id, const char * url, int conn_mode);
    void disable();
    int isConnect();
    bool changeURL(int id, const char* url, int conn_mode);
    bool reConnect(int id);

    struct FrameData * read_Opencv();

private:
    pthread_t m_thread;
    struct CustomData m_data;

};

#endif //SQ_RTSP_H
