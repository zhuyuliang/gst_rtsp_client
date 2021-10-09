//
// Created by steve 21-09-16
//
//  gstreamer如何接入RTSP流（IP摄像头） h264/h265
//  uridecodebin videoconvert autovideosink 

#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <glib/gstdio.h>
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
#include <gst/rtsp/gstrtsptransport.h>

// extern "C" {
//   #include <rockchip/rockchip_rga.h>
// }
#include <rga/rga.h>
#include <rga/RgaUtils.h>
#include <rga/RockchipRga.h>
#include <rga/im2d.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>
#include <stdio.h>
#include <unistd.h>



// gst log
GST_DEBUG_CATEGORY_STATIC (rk_appsink_debug);
#define GST_CAT_DEFAULT rk_appsink_debug

#define PNAME   "index0"
#define RTSPCAM "rtsp://admin:shangqu2020@192.168.2.30:554/cam/realmonitor?channel=1&subtype=0"
#define DISPLAY FALSE

//rga

#define SRC_WIDTH  1920
#define SRC_HEIGHT 1080
#define SRC_FORMAT RK_FORMAT_YCrCb_420_SP

#define DST_WIDTH 1920
#define DST_HEIGHT 1080
#define DST_FORMAT RK_FORMAT_RGB_888
#define BUFFER_SIZE DST_WIDTH*DST_HEIGHT*3

inline static const char *
yesno (int yes)
{
  return yes ? "yes" : "no";
}

// CustomData
struct CustomData {
    GMainLoop *loop;
    GstElement *pipeline;
    GstElement *rtspsrc;
    GstElement *decode;
    GstElement *tee;
    GstElement *queue_appsink;
    GstElement *queue_displaysink;
    GstElement *appsink;
    GstElement *displaysink;

    pthread_t gst_thread;

    gint format;
    GstVideoInfo info;

    unsigned frame;
};

static GstFlowReturn new_sample (GstElement *sink, GMainLoop *pipline) {
    GstSample *sample;

    g_signal_emit_by_name(sink,"pull-sample", &sample);
    if (sample) {
        g_print ("*");
        gst_sample_unref (sample);
        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}

// appsink probe
static GstPadProbeReturn
pad_probe (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
  struct CustomData *dec = (struct CustomData *) user_data;
  GstEvent *event = GST_PAD_PROBE_INFO_EVENT (info);
  GstCaps *caps;

  (void) pad;

  if (GST_EVENT_TYPE (event) != GST_EVENT_CAPS)
    return GST_PAD_PROBE_OK;

  gst_event_parse_caps (event, &caps);

  if (!caps) {
    GST_ERROR ("caps event without caps");
    return GST_PAD_PROBE_OK;
  }

  if (!gst_video_info_from_caps (&dec->info, caps)) {
    GST_ERROR ("caps event with invalid video caps");
    return GST_PAD_PROBE_OK;
  }

  switch (GST_VIDEO_INFO_FORMAT (&(dec->info))) {
    case GST_VIDEO_FORMAT_I420:
      dec->format = 2;
      break;
    case GST_VIDEO_FORMAT_NV12:
      dec->format = 23;
      break;
    case GST_VIDEO_FORMAT_YUY2:
      dec->format = 4;
      break;
    default:
      GST_ERROR ("unknown format\n");
      return GST_PAD_PROBE_OK;
  }

  return GST_PAD_PROBE_OK;
}

// uridecodebin -> src link tee -> sink 
static void 
on_src_tee_added(GstElement *element, GstPad *pad, gpointer data) {
    GstPad *sinkpad;
    struct CustomData *decoder = (struct CustomData *)data;

    /* We can now link this pad with the rtsp-decoder sink pad */
    g_print("Dynamic pad created, linking source/demuxer\n");
    sinkpad = gst_element_get_static_pad(decoder->tee, "sink");
    gst_pad_link(pad, sinkpad);
    gst_object_unref(sinkpad);
}

static void 
on_src_decodebin_added(GstElement *element, GstPad *pad, gpointer data) {
    GstPad *sinkpad;
    struct CustomData *decoder = (struct CustomData *)data;
    
    /* We can now link this pad with the rtsp-decoder sink pad */
    g_print("Dynamic pad created, linking source/decodeon_src_decodebin_added\n");
    sinkpad = gst_element_get_static_pad(decoder->decode, "sink");
    gst_pad_link(pad, sinkpad);
    gst_object_unref(sinkpad);
}

// appsink query
static GstPadProbeReturn
appsink_query_cb (GstPad * pad G_GNUC_UNUSED, GstPadProbeInfo * info,
    gpointer user_data G_GNUC_UNUSED)
{
  GstQuery *query = (GstQuery *) info->data;

  if (GST_QUERY_TYPE (query) != GST_QUERY_ALLOCATION)
    return GST_PAD_PROBE_OK;

  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);

  return GST_PAD_PROBE_HANDLED;
}

// bus
static gboolean
bus_watch_cb (GstBus * bus, GstMessage * msg, gpointer user_data)
{
  struct CustomData *dec = (struct CustomData *) user_data;
  
  //param not use
  (void) bus;

  //g_print ( "piple call_bus message %d \n",msg->type);

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_STATE_CHANGED:{
      gchar *dotfilename;
      GstState old_gst_state, cur_gst_state, pending_gst_state;

      /* Only consider state change messages coming from
       * the toplevel element. */
      if (GST_MESSAGE_SRC (msg) != GST_OBJECT (dec->pipeline))
        break;

      gst_message_parse_state_changed (msg, &old_gst_state, &cur_gst_state,
          &pending_gst_state);

      printf ("GStreamer state change:  old: %s  current: %s  pending: %s\n",
          gst_element_state_get_name (old_gst_state),
          gst_element_state_get_name (cur_gst_state),
          gst_element_state_get_name (pending_gst_state)
          );

      dotfilename = g_strdup_printf ("statechange__old-%s__cur-%s__pending-%s",
          gst_element_state_get_name (old_gst_state),
          gst_element_state_get_name (cur_gst_state),
          gst_element_state_get_name (pending_gst_state)
          );
      GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (dec->pipeline),
          GST_DEBUG_GRAPH_SHOW_ALL, dotfilename);
      g_free (dotfilename);

      break;
    }
    case GST_MESSAGE_REQUEST_STATE:{
      GstState requested_state;
      gst_message_parse_request_state (msg, &requested_state);
      printf ("state change to %s was requested by %s\n",
          gst_element_state_get_name (requested_state),
          GST_MESSAGE_SRC_NAME (msg)
          );
      gst_element_set_state (GST_ELEMENT (dec->pipeline), requested_state);
      break;
    }
    case GST_MESSAGE_LATENCY:{
      printf ("redistributing latency\n");
      gst_bin_recalculate_latency (GST_BIN (dec->pipeline));
      break;
    }
    case GST_MESSAGE_EOS:
      g_print("bus eos \n");
      g_main_loop_quit (dec->loop);

      break;
    case GST_MESSAGE_INFO:
    case GST_MESSAGE_WARNING:
    case GST_MESSAGE_ERROR:{
      g_print ( "piple call_bus message \n");
      GError *error = NULL;
      gchar *debug_info = NULL;
      gchar const *prefix = "";

      switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_INFO:
          gst_message_parse_info (msg, &error, &debug_info);
          prefix = "INFO";
          break;
        case GST_MESSAGE_WARNING:
          gst_message_parse_warning (msg, &error, &debug_info);
          prefix = "WARNING";
          break;
        case GST_MESSAGE_ERROR:
          gst_message_parse_error (msg, &error, &debug_info);
          prefix = "ERROR";
          break;
        default:
          g_assert_not_reached ();
      }
      g_print ("GStreamer %s: %s; debug info: %s", prefix, error->message,
          debug_info);

      g_clear_error (&error);
      g_free (debug_info);

      if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR) {
        GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (dec->pipeline),
            GST_DEBUG_GRAPH_SHOW_ALL, "error");
      }
      // TODO: stop mainloop in case of an error
      g_main_loop_quit(dec->loop);

      break;
    }
    default:
      break;
  }

  return TRUE;
}

static void *
buffer_to_file (struct CustomData *dec, GstBuffer * buf,  char *srcBuffer_264,char *srcBuffer_265,  char *dstBuffer)
{
  int ret;
  GstVideoMeta *meta = gst_buffer_get_video_meta (buf);
  guint nplanes = GST_VIDEO_INFO_N_PLANES (&(dec->info));
  guint width, height;
  GstMapInfo map_info;
  gchar filename[128];
  GstVideoFormat pixfmt;
  const char *pixfmt_str;

  pixfmt = GST_VIDEO_INFO_FORMAT (&(dec->info));
  pixfmt_str = gst_video_format_to_string (pixfmt);

  /* TODO: use the DMABUF directly */

  gst_buffer_map (buf, &map_info, GST_MAP_READ);

  width = GST_VIDEO_INFO_WIDTH (&(dec->info));
  height = GST_VIDEO_INFO_HEIGHT (&(dec->info));

  /* output some information at the beginning (= when the first frame is handled) */
  if (dec->frame == 0) {
    printf ("===================================\n");
    printf ("GStreamer video stream information:\n");
    printf ("  size: %u x %u pixel\n", width, height);
    printf ("  pixel format: %s  number of planes: %u\n", pixfmt_str, nplanes);
    printf ("  video meta found: %s\n", yesno (meta != NULL));
    printf ("===================================\n");
  }

  g_print( "*");

  // int resize_w = 1920, resize_h = 1080;
  // static int frame_size = 0;
  // unsigned char *frame_rgb = NULL;
  // frame_size = resize_w * resize_h * 3;
  // frame_rgb = (unsigned char *)malloc(frame_size);
  // //cv::Mat img(resize_h , resize_w , CV_8UC3, frame_rgb);
  // if (!frame_rgb)
  //   return 0;
    
  // mRga->ops->setDstBufferPtr(mRga, frame_rgb);

  // // g_print( "*Rga \n");
  // // //memcpy((char *) map_info.data, srcBuffer, BUFFER_SIZE);
  // mRga->ops->setSrcBufferPtr(mRga, (char *) map_info.data);
  // // mRga->ops->setDstBufferPtr(mRga, dstBuffer);
  rga_buffer_t 	src;
  rga_buffer_t 	dst;
  rga_buffer_t  dst_output;

  // mpp 265 256 2304 | 264 16 1088

  if (map_info.size == 4976640){
    // g_print("h265");
    src = wrapbuffer_virtualaddr((char *) map_info.data, 2304, SRC_HEIGHT, SRC_FORMAT);
    dst = wrapbuffer_virtualaddr(srcBuffer_265, 2304, DST_HEIGHT, DST_FORMAT);
  } else {
    src = wrapbuffer_virtualaddr((char *) map_info.data, SRC_WIDTH, 1088, SRC_FORMAT);
    dst = wrapbuffer_virtualaddr(srcBuffer_264, DST_WIDTH, 1088, DST_FORMAT);
  }
  // dst = wrapbuffer_virtualaddr(srcBuffer_264, DST_WIDTH, DST_WIDTH, DST_FORMAT);
  dst_output = wrapbuffer_virtualaddr(dstBuffer, DST_WIDTH, DST_HEIGHT, DST_FORMAT);

  if(src.width == 0 || dst.width == 0 || dst_output.width == 0) {
    printf("%s, %s\n", __FUNCTION__, imStrError());
    return 0;
  }

  // g_print("imcvtcolor");

  // ret = mRga->ops->go(mRga);
  imcvtcolor(src, dst, src.format, dst.format);

  im_rect src_rect = {0, 0, DST_WIDTH, DST_HEIGHT};
  //g_print("imcrop %d",src_rect.width);
  imcrop(dst,dst_output,src_rect);

  cv::Mat img(DST_HEIGHT, DST_WIDTH , CV_8UC3, dstBuffer);
  char buf1[32] = {0};
  snprintf(buf1, sizeof(buf1), "%u",dec->frame);
  std::string str = buf1;
  std::string name = str + "test.jpg";
  
  // cv::imwrite( name, img);
  // cv::waitKey(10);

  // g_print("num: %d \n", checkData(srcBuffer,dstBuffer));

  // cv::Mat img(BUFFER_HEIGHT, BUFFER_WIDTH , CV_8UC3, dstBuffer);
  // cv::imwrite("test.jpg", img); 
  // //unchar_to_Mat(dstBuffer); 
  // g_print( "*Rga ret = %d \n",ret);
  // if (!ret) {
	// 		///do something with frame_rgb
	// 		cv::imwrite("test.jpg", img);
	// 		cv::waitKey(10);
	// }

  //unchar_to_Mat(dstBuffer);  // 将 unsigned char BGR格式转换为 Mat BGR格式
  //cv::imwrite
  //cv::imshow("image",img);
  //cv::waitKey(20);

  // g_snprintf (filename, sizeof (filename), "img%05d.%s", dec->frame,
  //     pixfmt_str);
  // g_file_set_contents (filename, (char *) map_info.data, map_info.size, NULL);
  

  gst_buffer_unmap (buf, &map_info);

  return 0;
}

static void *
video_frame_loop (void *arg)
{
  struct CustomData *dec = (struct CustomData *) arg;

  char* src_buf_264 = NULL;
  char* src_buf_265 = NULL;
  char* dst_buf = NULL;

  src_buf_264 = (char*)malloc(SRC_WIDTH*1088*get_bpp_from_format(DST_FORMAT));
  src_buf_265 = (char*)malloc(2304*SRC_HEIGHT*get_bpp_from_format(DST_FORMAT));
  dst_buf = (char*)malloc(DST_WIDTH*DST_HEIGHT*get_bpp_from_format(DST_FORMAT));

  //mRga->ops->setSrcBufferPtr(mRga, srcBuffer);
  //mRga->ops->setDstBufferPtr(mRga, dstBuffer);
  gst_app_sink_set_max_buffers (GST_APP_SINK(dec->appsink),1);
  gst_app_sink_set_buffer_list_support (GST_APP_SINK(dec->appsink),FALSE);

  //h264 h265 1920  (other inch no support)

  do {
    GstSample *samp;
    GstBuffer *buf;

    samp = gst_app_sink_pull_sample (GST_APP_SINK (dec->appsink));
    // samp = gst_app_sink_try_pull_sample (GST_APP_SINK (dec->appsink),100000);
    if (!samp) {
      GST_DEBUG ("got no appsink sample");
      if (gst_app_sink_is_eos (GST_APP_SINK (dec->appsink)))
        GST_DEBUG ("eos");
      return NULL;
    }

    buf = gst_sample_get_buffer (samp);
    buffer_to_file (dec, buf, src_buf_264,src_buf_265, dst_buf);

    gst_sample_unref (samp);
    dec->frame++;

    // sleep(2);

  } while (1);

  g_free(src_buf_264);
  g_free(src_buf_265);
  g_free(dst_buf);

}

// rtsp init
static struct CustomData *
rtsp_init(const char *pname, const char *rtsp_uri) {

    struct CustomData *data;
    data = g_new0 (struct CustomData, 1);

    GstBus *bus;
    GstPad *apppad;
    GstPad *queue1_video_pad;
    GstPad *queue2_video_pad;
    GstPad *tee1_video_pad;
    GstPad *tee2_video_pad;

    /* Build Pipeline */
    data->pipeline = gst_pipeline_new(pname);

    data->rtspsrc = gst_element_factory_make ( "rtspsrc", "rtspsrc0");
    data->decode  = gst_element_factory_make ( "decodebin", "decodebin0");
    //data->mppdec = gst_element_factory_make ( "mppvideodec", "mppvideodec0");
    data->tee    = gst_element_factory_make ( "tee", "tee0");

    if (DISPLAY) {
        data->queue_displaysink  = gst_element_factory_make ( "queue", "queue_displaysink0");
        data->displaysink      = gst_element_factory_make ( "rkximagesink", "display_sink0");
    }
    data->queue_appsink      = gst_element_factory_make ( "queue", "queue_appsink0");
    data->appsink            = gst_element_factory_make ( "appsink", "app_sink0");

    if (!DISPLAY) {
        if ( !data->pipeline || !data->rtspsrc | !data->decode || !data->tee || !data->queue_appsink || !data->appsink) {
            g_printerr("One element could not be created.\n");
        }
    }else{
        if ( !data->pipeline || !data->rtspsrc | !data->decode || !data->tee || !data->queue_appsink || !data->queue_displaysink  || !data->displaysink || !data->appsink) {
            g_printerr("One element could not be created.\n");
        }
        // Configure rksink
        g_object_set (G_OBJECT (data->displaysink), "sync", FALSE, NULL);
    }

    // Config appsink
    g_object_set (G_OBJECT (data->appsink), "sync", FALSE, NULL);
    /* Implement the allocation query using a pad probe. This probe will
    * adverstize support for GstVideoMeta, which avoid hardware accelerated
    * decoder that produce special strides and offsets from having to
    * copy the buffers.
    */
    apppad = gst_element_get_static_pad (data->appsink, "sink");
    gst_pad_add_probe (apppad, GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM,
        appsink_query_cb, NULL, NULL);
    gst_object_unref (apppad);

    gst_base_sink_set_max_lateness (GST_BASE_SINK (data->appsink), 70 * GST_MSECOND);
    gst_base_sink_set_qos_enabled (GST_BASE_SINK (data->appsink), TRUE);

    g_object_set (G_OBJECT (data->appsink), "max-buffers", 1, NULL);

    gst_pad_add_probe (gst_element_get_static_pad (data->appsink, "sink"),
      GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, pad_probe, data, NULL);
    g_object_set (G_OBJECT (data->appsink), "emit-signals", TRUE, NULL );
    // g_signal_connect ( G_OBJECT (data->appsink), "new-sample", G_CALLBACK (new_sample), data->pipeline);

    //create_uri(url,url_size, ip_address, port);
    g_object_set(GST_OBJECT(data->rtspsrc), "location", RTSPCAM, NULL);
    g_object_set (GST_OBJECT (data->rtspsrc), "latency", 2000, NULL);
    g_object_set (GST_OBJECT (data->rtspsrc), "timeout", 1000000, NULL);
    g_object_set (GST_OBJECT (data->rtspsrc), "udp-reconnect", 1, NULL);
    g_object_set (GST_OBJECT (data->rtspsrc), "retry", 1000, NULL);
    g_object_set (GST_OBJECT (data->rtspsrc), "debug", 1, NULL);

    /**
     * GstRTSPLowerTrans:
     * @GST_RTSP_LOWER_TRANS_UNKNOWN: invalid transport flag
     * @GST_RTSP_LOWER_TRANS_UDP: stream data over UDP
     * @GST_RTSP_LOWER_TRANS_UDP_MCAST: stream data over UDP multicast
     * @GST_RTSP_LOWER_TRANS_TCP: stream data over TCP
     * @GST_RTSP_LOWER_TRANS_HTTP: stream data tunneled over HTTP.
     * @GST_RTSP_LOWER_TRANS_TLS: encrypt TCP and HTTP with TLS
     *
     * The different transport methods.
     */
    //g_object_set (GST_OBJECT (data->rtspsrc), "protocols", GST_RTSP_LOWER_TRANS_UDP, NULL);
    //"rtsp://<ip>:554/live/ch00_0"

    g_object_set (G_OBJECT (data->queue_appsink), "max-size-buffers", 1, NULL);
    // g_object_set (G_OBJECT (data->queue_appsink), "max-size-bytes", 1000, NULL);
    g_object_set (G_OBJECT (data->queue_appsink), "leaky", 2, NULL);

    if (DISPLAY) {
        gst_bin_add_many(GST_BIN(data->pipeline), data->queue_displaysink, data->displaysink, NULL);
        // queue -> rkximagesink
        if (!gst_element_link_many (data->queue_displaysink, data->displaysink, NULL)) {
            g_printerr ("Elements could not be linked.\n");
            gst_object_unref (data->pipeline);
            return NULL;
        }
    }
    gst_bin_add_many(GST_BIN(data->pipeline), data->rtspsrc, data->decode, data->tee, NULL);
    gst_bin_add_many(GST_BIN(data->pipeline), data->queue_appsink, data->appsink, NULL);
    // gst_bin_add_many(GST_BIN(data->pipeline), data->queue_appsink, data->videoconvert, data->appsink, NULL);

    // queue -> rkximagesink  ||  queue -> appsink
    if (!gst_element_link_many (data->queue_appsink, data->appsink, NULL)) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (data->pipeline);
        return NULL;
    }

    if (DISPLAY) {
        queue1_video_pad = gst_element_get_static_pad ( data->queue_displaysink, "sink");
        tee1_video_pad = gst_element_get_request_pad ( data->tee, "src_%u");
        if (gst_pad_link ( tee1_video_pad, queue1_video_pad) != GST_PAD_LINK_OK) {
            g_printerr ("tee link queue error. \n");
            gst_object_unref (data->pipeline);
            return NULL;
        }
        gst_object_unref (queue1_video_pad);
        gst_object_unref (tee1_video_pad);
    }
    //tee -> queue1 -> queue2
    queue2_video_pad = gst_element_get_static_pad ( data->queue_appsink, "sink");
    tee2_video_pad = gst_element_get_request_pad ( data->tee, "src_%u");
    if (gst_pad_link ( tee2_video_pad, queue2_video_pad) != GST_PAD_LINK_OK) {
        g_printerr ("tee link queue error. \n");
        gst_object_unref (data->pipeline);
        return NULL;
    }
    gst_object_unref (queue2_video_pad);
    gst_object_unref (tee2_video_pad);

    g_signal_connect(data->rtspsrc, "pad-added", G_CALLBACK( on_src_decodebin_added), data);
    g_signal_connect(data->decode, "pad-added", G_CALLBACK( on_src_tee_added), data);
    
    bus = gst_pipeline_get_bus( GST_PIPELINE (data->pipeline));
    gst_bus_add_watch (bus, bus_watch_cb, data);
    gst_object_unref ( GST_OBJECT (bus));

    // bus = gst_element_get_bus( GST_ELEMENT (data->appsink));
    // gst_bus_add_watch (bus, (GstBusFunc) bus_uridecodebin_cb, data);
    // gst_object_unref ( GST_OBJECT (bus));

    // start playing
    g_print ("start playing \n");
    gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
    data->loop = g_main_loop_new (NULL, FALSE);
    //g_main_loop_run (main_loop);

    pthread_create (&data->gst_thread, NULL, video_frame_loop, data);

    return data;
}

// destroy
static void
rtsp_destroy (struct CustomData *data)
{
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    pthread_join (data->gst_thread, 0);

    gst_bin_remove_many (GST_BIN(data->pipeline), data->rtspsrc, NULL);

    gst_bin_remove_many (GST_BIN(data->pipeline), data->decode, data->tee, NULL);
    //gst_object_unref (data->source);
    //gst_object_unref (data->tee);
    if (DISPLAY) {
      gst_bin_remove_many (GST_BIN(data->pipeline), data->queue_displaysink, data->displaysink, NULL);
      //gst_object_unref (data->queue_displaysink);
      //gst_object_unref (data->displaysink);
    }
    gst_bin_remove_many (GST_BIN(data->pipeline), data->queue_appsink, data->appsink, NULL);
    //gst_object_unref (data->queue_appsink);
    //gst_object_unref (data->appsink);

    gst_object_unref (data->pipeline);
    g_main_loop_quit (data->loop);
    g_main_loop_unref (data->loop);

    g_free (data);

    sleep(3);
}

// main
int main(int argc, char *argv[]) {

    // public customData
    struct CustomData *data = NULL;

    gst_init (&argc, &argv);
    GST_DEBUG_CATEGORY_INIT (rk_appsink_debug, "rk_appsink", 2, "App sink");

    do{

      data = rtsp_init (PNAME, RTSPCAM);

      g_main_loop_run (data->loop);

      rtsp_destroy (data);

    }while (1);

}