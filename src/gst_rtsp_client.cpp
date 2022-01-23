
#include <gst_rtsp_client.h>

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
#include <gst/rtsp/gstrtsptransport.h>

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

// #include <arrayqueue.h>

static GstFlowReturn 
new_sample (GstElement *sink, GMainLoop *pipline) {
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

  dec->isRun = STATUS_CONNECTED;

  return GST_PAD_PROBE_OK;
}

// uridecodebin -> src link tee -> sink 
static void 
on_src_tee_added(GstElement *element, GstPad *pad, gpointer data) {
    // GstPad *sinkpad;
    struct CustomData *decoder = (struct CustomData *)data;

    /* We can now link this pad with the rtsp-decoder sink pad */
    g_print("Dynamic pad created, linking source/demuxer\n");
    GstPad * tee_sinkpad = gst_element_get_static_pad(decoder->tee, "sink");
    gst_pad_link(pad, tee_sinkpad);
    gst_object_unref(tee_sinkpad);
}

static void 
on_src_decodebin_added(GstElement *element, GstPad *pad, gpointer data) {
    struct CustomData *decoder = (struct CustomData *)data;
    
    // decoder->isRun = STATUS_CONNECTED;
    /* We can now link this pad with the rtsp-decoder sink pad */
    g_print("Dynamic pad created, linking source/decodeon_src_decodebin_added\n");
    GstPad * decode_sinkpad = gst_element_get_static_pad(decoder->decode, "sink");
    gst_pad_link(pad, decode_sinkpad);
    gst_object_unref(decode_sinkpad);
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
  // (void) bus;

  // g_print ( "piple call_bus message m_Id %s %d \n", gst_message_type_get_name(GST_MESSAGE_TYPE (msg)),dec->m_Id);
  

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_UNKNOWN: {
      g_print ( "piple call_bus message GST_MESSAGE_UNKNOWN %d \n",dec->m_Id);
    }
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

      // g_print ( "piple call_bus message GST_MESSAGE_STATE_CHANGED %d \n",dec->m_Id);

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
      // g_main_loop_quit (dec->loop);
      dec->isRun = STATUS_DISCONNECT;
      break;
    case GST_MESSAGE_INFO:
    case GST_MESSAGE_WARNING:
    case GST_MESSAGE_ERROR:{
      g_print ( "piple call_bus message %d \n",dec->m_Id);
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
      // g_main_loop_quit(dec->loop);
      g_print("bus disconnect %d \n",dec->m_Id);
      dec->isRun = STATUS_DISCONNECT;

      break;
    }
    default:
      break;
  }

  return TRUE;
}

static void
buffer_to_file (struct CustomData *dec, GstBuffer * buf)
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

  if (dec->m_RtspCallBack != NULL)
  {
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
    // rga_buffer_t 	src;
    // rga_buffer_t 	dst;
    // rga_buffer_t  dst_output;

    // // mpp 265 256 2304 | 264 16 1088

    // if (map_info.size == 4976640){
    //   // g_print("h265");
    //   src = wrapbuffer_virtualaddr((char *) map_info.data, 2304, SRC_HEIGHT, SRC_FORMAT);
    //   dst = wrapbuffer_virtualaddr(srcBuffer_265, 2304, DST_HEIGHT, DST_FORMAT);
    // } else {
    //   src = wrapbuffer_virtualaddr((char *) map_info.data, SRC_WIDTH, 1088, SRC_FORMAT);
    //   dst = wrapbuffer_virtualaddr(srcBuffer_264, DST_WIDTH, 1088, DST_FORMAT);
    // }
    // // dst = wrapbuffer_virtualaddr(srcBuffer_264, DST_WIDTH, DST_WIDTH, DST_FORMAT);
    // dst_output = wrapbuffer_virtualaddr(dstBuffer, DST_WIDTH, DST_HEIGHT, DST_FORMAT);

    // if(src.width == 0 || dst.width == 0 || dst_output.width == 0) {
    //   printf("%s, %s\n", __FUNCTION__, imStrError());
    //   return;
    // }

    // // g_print("imcvtcolor");

    // // ret = mRga->ops->go(mRga);
    // imcvtcolor(src, dst, src.format, dst.format);

    // im_rect src_rect = {0, 0, DST_WIDTH, DST_HEIGHT};
    // //g_print("imcrop %d",src_rect.width);
    // imcrop(dst,dst_output,src_rect);

    // cv::Mat img(DST_HEIGHT, DST_WIDTH , CV_8UC3, dstBuffer);
    // char buf1[32] = {0};
    // snprintf(buf1, sizeof(buf1), "%u",dec->frame);
    // std::string str = buf1;
    // std::string name = str + "test.jpg";
    
    // cv::imwrite( name, img);
    // cv::waitKey(10);

    dec->m_RtspCallBack();

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
  } else {

    g_print("b* \n");

    // rga_buffer_t 	src;
    //     rga_buffer_t 	dst;
    //     rga_buffer_t  dst_output;
    //     rga_buffer_t  dst_resize_output;

    // // h264 h265 1280 * 720 == 1280 * 720
    //       src = wrapbuffer_virtualaddr((char *) map_info.data, 1280, 720, SRC_FORMAT);
    //       if (dec->dst_buf == NULL){
    //           dec->dst_buf = (char*)malloc(1280*720*get_bpp_from_format(DST_FORMAT));
    //       }
    //       dst = wrapbuffer_virtualaddr(dec->dst_buf, 1280, 720, DST_FORMAT);
    //       // if (this->m_data->dst_output_buf == NULL){
    //       //     this->m_data->dst_output_buf = (char*)malloc(1280*720*get_bpp_from_format(DST_FORMAT));
    //       // }
    //       // dst_output = wrapbuffer_virtualaddr(this->m_data->dst_output_buf, 1280, 720, DST_FORMAT);
    //       if (dec->dst_resize_output_buf == NULL){
    //           dec->dst_resize_output_buf = (char*)malloc(width*height*get_bpp_from_format(DST_FORMAT));
    //       }
    //       dst_resize_output = wrapbuffer_virtualaddr(dec->dst_resize_output_buf, width, height, DST_FORMAT);
    //       if(src.width == 0 || dst.width == 0 || dst_resize_output.width == 0) {
    //         printf("%s, %s\n", __FUNCTION__, imStrError());
    //         return;
    //       }
    //       imcvtcolor(src, dst, src.format, dst.format);
    //       //im_rect src_rect = {0, 0, 1280, 720};
    //       //g_print("imcrop %d",src_rect.width);
    //       //imcrop(dst,dst_output,src_rect);

    //       // imresize(dst,dst_resize_output);

    //       cv::Mat img(720, 1280 , CV_8UC3, dec->dst_buf);
    //       std::string name = "i10st.jpg";
            
    //       cv::imwrite( name, img);


    // g_print("queue empty %d" , dec->mqueue.empty());
    // FrameData framedata;
    // framedata.data = (char *) map_info.data;
    // framedata.size = map_info.size;
    // framedata.width = width;
    // framedata.height = height;
    // framedata.isRun = dec->isRun;

    // MppFrameData mppframeData;
    // //memcpy(mppframeData.data,(char *)map_info.data, map_info.size);
    // // mppframeData.data = (char *)map_info.data;
    // mppframeData.size = map_info.size;
    // if (dec->mqueue->is_empty()){
    //   // g_print("queue empty \n");
    //   dec->mqueue->add(mppframeData);
    // }else if (dec->mqueue->size() < 3){
    //   // g_print("queue size < 2 \n");
    //   dec->mqueue->add(mppframeData);
    // } else {
    //   // while(dec->mqueue->size())
    //   // g_print(">2 pop \n");
    //   // MppFrameData datas = 
    //   dec->mqueue->pop();
    // }
  }

  gst_buffer_unmap (buf, &map_info);

  return;
}

static void *
video_frame_loop (void *arg)
{
  struct CustomData *dec = (struct CustomData *) arg;

  //mRga->ops->setSrcBufferPtr(mRga, srcBuffer);
  //mRga->ops->setDstBufferPtr(mRga, dstBuffer);
  // gst_app_sink_set_max_buffers (GST_APP_SINK(dec->appsink),1);
  // gst_app_sink_set_buffer_list_support (GST_APP_SINK(dec->appsink),FALSE);

  // //h264 h265 1920  (other inch no support)
  // dec->mqueue = new ArrayQueue<MppFrameData>();

  do {
    GstSample *samp;
    GstBuffer *buf;

    samp = gst_app_sink_pull_sample (GST_APP_SINK (dec->appsink));
    // samp = gst_app_sink_try_pull_sample (GST_APP_SINK (dec->appsink),100000);
    if (!samp) {
      GST_DEBUG ("got no appsink sample");
      if (gst_app_sink_is_eos (GST_APP_SINK (dec->appsink)))
        GST_DEBUG ("eos");
      return((void *)0);
    }

    buf = gst_sample_get_buffer (samp);
    buffer_to_file (dec, buf);

    gst_sample_unref (samp);
    dec->frame++;

    // sleep(2);

  } while (1);

  return((void *)0);

}

// rtsp init
static struct CustomData *
rtsp_init(struct CustomData *data) {

    // GstBus *bus;
    // GstPad *apppad;
    // GstPad *queue1_video_pad;
    // GstPad *queue2_video_pad;
    // GstPad *tee1_video_pad;
    // GstPad *tee2_video_pad;

    /* Build Pipeline */
    data->pipeline = gst_pipeline_new(std::to_string(data->m_Id).c_str());

    data->rtspsrc = gst_element_factory_make ( "rtspsrc", "rtspsrc0");
    data->decode  = gst_element_factory_make ( "decodebin3", "decodebin0");
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
    GstPad * apppad = gst_element_get_static_pad (data->appsink, "sink");
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
    g_print("rtsp_uri:%s\n",data->m_RtspUri);
    g_object_set(GST_OBJECT(data->rtspsrc), "location", data->m_RtspUri, NULL);
    g_object_set (GST_OBJECT (data->rtspsrc), "latency", 2000, NULL);
    // g_object_set (GST_OBJECT (data->rtspsrc), "timeout", 1000000, NULL);
    // g_object_set (GST_OBJECT (data->rtspsrc), "udp-reconnect", 1, NULL);
    // g_object_set (GST_OBJECT (data->rtspsrc), "retry", 1000, NULL);
    // g_object_set (GST_OBJECT (data->rtspsrc), "debug", 1, NULL);

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
    // g_object_set (GST_OBJECT (data->rtspsrc), "protocols", GST_RTSP_LOWER_TRANS_HTTP, NULL);
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
        GstPad * queue1_video_pad = gst_element_get_static_pad ( data->queue_displaysink, "sink");
        GstPad * tee1_video_pad = gst_element_get_request_pad ( data->tee, "src_%u");
        if (gst_pad_link ( tee1_video_pad, queue1_video_pad) != GST_PAD_LINK_OK) {
            g_printerr ("tee link queue error. \n");
            gst_object_unref (data->pipeline);
            return NULL;
        }
        gst_object_unref (queue1_video_pad);
        gst_object_unref (tee1_video_pad);
    }
    //tee -> queue1 -> queue2
    GstPad * queue2_video_pad = gst_element_get_static_pad ( data->queue_appsink, "sink");
    GstPad * tee2_video_pad = gst_element_get_request_pad ( data->tee, "src_%u");
    if (gst_pad_link ( tee2_video_pad, queue2_video_pad) != GST_PAD_LINK_OK) {
        g_printerr ("tee link queue error. \n");
        gst_object_unref (data->pipeline);
        return NULL;
    }
    gst_object_unref (queue2_video_pad);
    gst_object_unref (tee2_video_pad);

    g_signal_connect(data->rtspsrc, "pad-added", G_CALLBACK( on_src_decodebin_added), data);
    g_signal_connect(data->decode, "pad-added", G_CALLBACK( on_src_tee_added), data);
    
    data->bus = gst_pipeline_get_bus( GST_PIPELINE (data->pipeline));
    gst_bus_add_watch (data->bus, bus_watch_cb, data);
    // gst_object_unref ( GST_OBJECT (bus));

    // bus = gst_element_get_bus( GST_ELEMENT (data->appsink));
    // gst_bus_add_watch (bus, (GstBusFunc) bus_uridecodebin_cb, data);
    // gst_object_unref ( GST_OBJECT (bus));

    gst_app_sink_set_max_buffers (GST_APP_SINK(data->appsink),1);
    gst_app_sink_set_buffer_list_support (GST_APP_SINK(data->appsink),FALSE);

    //h264 h265 1920  (other inch no support)
    // data->mqueue = new ArrayQueue<MppFrameData>();

    // start playing
    g_print ("start playing \n");
    gst_element_set_state (GST_ELEMENT(data->pipeline), GST_STATE_PLAYING);

    if (data->m_RtspCallBack != NULL){
      //g_main_loop_run (main_loop);
      pthread_create (&data->gst_thread, NULL, video_frame_loop, data);
    }
    
    return data;
}

// destroy
static void 
rtsp_destroy (pthread_t m_thread, struct CustomData *data)
{
  if (data != NULL) {
    // g_print ("start gst_bus_remove_watch \n");
    gst_bus_remove_watch(data->bus);
    gst_object_unref(data->bus);

    gst_element_set_state (GST_ELEMENT(data->pipeline), GST_STATE_PAUSED);
    gst_element_set_state (GST_ELEMENT(data->pipeline), GST_STATE_NULL);
    // if (DISPLAY) {
    //   gst_bin_remove_many (GST_BIN(data->pipeline), data->queue_displaysink, data->displaysink, NULL);
    //   gst_object_unref (data->queue_displaysink);
    //   gst_object_unref (data->displaysink);
    // }
    
    // g_print ("start gst_pad_unlink \n");
    // gst_pad_unlink(data->tee2_video_pad, data->queue2_video_pad);
    // gst_element_remove_pad(GST_ELEMENT(data->tee),data->tee2_video_pad);
    // gst_element_remove_pad(GST_ELEMENT(data->queue_appsink),data->queue2_video_pad);

    // g_print ("start gst_bin_remove_many \n");
    gst_element_unlink_many(data->appsink,data->queue_appsink,data->tee,data->decode,data->rtspsrc);
    // gst_element_unlink_many(data->queue_appsink,data->tee);
    // gst_element_unlink_many(data->tee,data->decode);
    // gst_element_unlink_many(data->decode,data->rtspsrc);

    gst_bin_remove_many(GST_BIN(data->pipeline), data->rtspsrc, data->decode, data->tee, NULL);
    gst_bin_remove_many(GST_BIN(data->pipeline), data->queue_appsink, data->appsink, NULL);

    // g_print ("start gst_object_unref3 \n");
    gst_object_unref (data->pipeline);

    // g_print ("start gst_object_unref2 \n");
    // gst_object_unref (data->appsink);
    // gst_object_unref (data->queue_appsink);
    // gst_object_unref (data->tee);
    // gst_object_unref (data->decode);
    // gst_object_unref (data->rtspsrc);

    // g_print ("start g_main_loop_quit \n");
    g_main_loop_quit (data->loop);
    g_main_loop_unref (data->loop);

    pthread_join (data->gst_thread, 0);
    pthread_detach (data->gst_thread);

    pthread_join (m_thread, 0);
    pthread_detach (m_thread);

    // g_print ("start == NULL \n");
    data->loop = NULL;
    data->pipeline = NULL;
    data->rtspsrc = NULL;
    data->decode = NULL;
    data->tee = NULL;
    data->queue_appsink = NULL;
    data->queue_displaysink = NULL;
    data->appsink = NULL;
    data->displaysink = NULL;

    data->bus = NULL;
    // data->apppad = NULL;
    // data->queue2_video_pad = NULL;
    // data->tee2_video_pad = NULL;

    // data->frame = NULL;
    // data->format = NULL;
    // data->gst_thread = NULL;
    // data->info = NULL;
    // data->m_Id = NULL;
    // data->m_RtspCallBack = NULL;
    // data->isRun = NULL;

    // g_print ("start g_free \n");
    delete (data->dst_buf);
    delete (data->dst_output_buf);
    delete (data->dst_resize_output_buf);
    delete (data->m_RtspUri);

    delete (data->dst_output_resize_buf);
    delete (data->dst_resize_output_resize_buf);

    // delete loop;
    // delete pipeline;
    // delete rtspsrc;
    // delete decode;
    // delete tee;
    // delete queue_appsink;
    // delete queue_displaysink;
    // delete appsink;
    // delete displaysink; 
    // g_print ("start pthread_quite \n");

    delete (data);
    
  }

}

RtspClient::RtspClient() {
    g_print("rtsp rtspclient\n");
}

RtspClient::~RtspClient() {
    g_print("rtsp ~rtspclient!\n");
}

static void* connectrtsp(void *arg) {

  struct CustomData *data = (struct CustomData *)arg;
  
  // gst init
  gst_init (NULL, NULL);
  /** gst debug */
  //GST_DEBUG_CATEGORY_INIT (rk_appsink_debug, "rk_appsink", 2, "App sink");
  g_print("mId %d mRtspUri %s \n", data->m_Id, data->m_RtspUri);

  rtsp_init (data);

  if (data != NULL) {
    data->isRun = STATUS_CONNECTING;
    data->loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (data->loop);
  } else {
    g_print("enable connect fail \n");
  }

  // rtsp_destroy(data);
  // g_main_loop_unref(data->loop);

  g_print("init done");
  data->isRun = STATUS_DISCONNECT;

  return 0;

}

bool
RtspClient::enable(int id, const char * url, int urllen, FRtspCallBack frtspcallback) {

  if (this->m_data == NULL) {
      this->m_data = g_new0 (struct CustomData, 1);
      this->m_data->m_Id = id;
      this->m_data->url_size = urllen;
      this->m_data->m_RtspUri = (char *)malloc(urllen);
      memcpy(this->m_data->m_RtspUri, url, urllen);
      // this->m_data->m_RtspUri = url;
      this->m_data->m_RtspCallBack = frtspcallback;
      int ret = pthread_create(&m_thread, NULL, connectrtsp, this->m_data);
      if ( ret != 0) {
        g_error("enable fail, rtsp thread create fail \n");
        return FALSE;
      }
  } else {
      g_error("enable fail, rtsp thread running \n");
      return FALSE;
  } 

  return TRUE;

}

bool
RtspClient::enable(int id, const char * url, int urllen) {

  if (this->m_data == NULL) {
      this->m_data = g_new0 (struct CustomData, 1);
      this->m_data->m_Id = id;
      this->m_data->url_size = urllen;
      this->m_data->m_RtspUri = (char *)malloc(urllen);
      memcpy(this->m_data->m_RtspUri, url, urllen);
      // this->m_data->m_RtspUri = url;
      int ret = pthread_create(&m_thread, NULL, connectrtsp, this->m_data);
      if ( ret != 0) {
        g_error("enable fail, rtsp thread create fail \n");
        return FALSE;
      }
  } else {
      g_error("enable fail, rtsp thread running \n");
      return FALSE;
  } 

  return TRUE;

}

void 
RtspClient::disable() {

    g_print("RtspClient start ~disable!\n");
    rtsp_destroy(this->m_thread, this->m_data);
    g_print("RtspClient end ~disable!\n");

}

int RtspClient::isConnect() 
{
  if (this->m_data == NULL){
    return STATUS_DISCONNECT;
  }
  return this->m_data->isRun;
}

// int
// RtspClient::reconnect() 
// {
//   if (this->m_data == NULL){
//     g_print("reconnected \n");
//     return STATUS_INIT;
//   }else
//   if (this->m_data->isRun == STATUS_DISCONNECT){
//     g_print("reconnected  STATUS_DISCONNECT \n");
//     int index = this->m_data->m_Id;
//     int url_size = this->m_data->url_size;
//     char * rtspurl = (char *)malloc(this->m_data->url_size);
//     memcpy(rtspurl, this->m_data->m_RtspUri, this->m_data->url_size);
//     this->disable();
//     int ret = this->enable(index, rtspurl, url_size, this->m_data->m_RtspCallBack);
//     g_free(rtspurl);
//     return ret;
//   } else {
//     g_print("reconnected \n");
//     return this->m_data->isRun;
//   }

// }

struct FrameData *
RtspClient::read(int width, int height,int resize_width, int resize_height) {

  FrameData * data = new FrameData();

  if (this->m_data == NULL || this->m_data->isRun == STATUS_DISCONNECT){
    data->isRun = STATUS_DISCONNECT;
    data->size = 0;
    return data;
  }

  data->isRun = this->m_data->isRun;
  data->size = 0;
  data->width = this->m_data->info.width;
  data->height = this->m_data->info.height;

  GstSample *samp;
  GstBuffer *buf;

  samp = gst_app_sink_pull_sample (GST_APP_SINK (this->m_data->appsink));
  // samp = gst_app_sink_try_pull_sample (GST_APP_SINK (dec->appsink),100000);
  if (!samp) {
    GST_DEBUG ("got no appsink sample");
    if (gst_app_sink_is_eos (GST_APP_SINK (this->m_data->appsink)))
      GST_DEBUG ("eos");
      return data;
  }

  buf = gst_sample_get_buffer (samp);
  
  // ** 

  // int ret;
  // GstVideoMeta *meta = gst_buffer_get_video_meta (buf);
  // guint nplanes = GST_VIDEO_INFO_N_PLANES (&(this->m_data->info));
  // guint width, height;
  GstMapInfo map_info;
  // gchar filename[128];
  // GstVideoFormat pixfmt;
  // const char *pixfmt_str;

  // pixfmt = GST_VIDEO_INFO_FORMAT (&(this->m_data->info));
  // pixfmt_str = gst_video_format_to_string (pixfmt);

  /* TODO: use the DMABUF directly */
  gst_buffer_map (buf, &map_info, GST_MAP_READ);

  int source_width = GST_VIDEO_INFO_WIDTH (&(this->m_data->info));
  int source_height = GST_VIDEO_INFO_HEIGHT (&(this->m_data->info));

  /* output some information at the beginning (= when the first frame is handled) */
  // if (this->m_data->frame == 0) {
  //   printf ("===================================\n");
  //   printf ("GStreamer video stream information:\n");
  //   printf ("  size: %u x %u pixel\n", source_width, source_height);
  //   printf ("  pixel format: %s  number of planes: %u\n", pixfmt_str, nplanes);
  //   printf ("  video meta found: %s\n", yesno (meta != NULL));
  //   printf ("===================================\n");
  // }

  // rga
  rga_buffer_t 	src;
  rga_buffer_t 	dst;
  rga_buffer_t  dst_output;
  rga_buffer_t  dst_resize_output;
  rga_buffer_t  dst_two_resize_output;
  rga_buffer_t  dst_two_output;

  // g_print("mppframe size : %d \n", map_info.size);

  // g_print("format %f \n",get_bpp_from_format(RK_FORMAT_YCrCb_420_SP));

  // // 1920*1080
  // // mpp 265 256 2304 | 264 16 1088
  if (map_info.size == 3732480 || map_info.size == 4976640 ) {
    // h265 256bit 1920 * 1080  == 2304 * 1080
    src = wrapbuffer_virtualaddr((char *) map_info.data, 2304, 1080, SRC_FORMAT);
    if (this->m_data->dst_buf == NULL){
      this->m_data->dst_buf = (char*)malloc(2304*1080*get_bpp_from_format(DST_FORMAT));
    }
    dst = wrapbuffer_virtualaddr(this->m_data->dst_buf, 2304, 1080, DST_FORMAT);
    if (this->m_data->dst_output_buf == NULL){
        this->m_data->dst_output_buf = (char*)malloc(1920*1080*get_bpp_from_format(DST_FORMAT));
    }
    dst_output = wrapbuffer_virtualaddr(this->m_data->dst_output_buf, 1920, 1080, DST_FORMAT);
    if (this->m_data->dst_resize_output_buf == NULL){
        this->m_data->dst_resize_output_buf = (char*)malloc(width*height*get_bpp_from_format(DST_FORMAT));
    }
    dst_resize_output = wrapbuffer_virtualaddr(this->m_data->dst_resize_output_buf, width, height,DST_FORMAT);
    if(src.width == 0 || dst.width == 0 || dst_output.width == 0) {
      printf("%s, %s\n", __FUNCTION__, imStrError());
      // return data;
    } else {
      // g_print("1080 h265 imcvtcolor \n");
      imcvtcolor(src, dst, src.format, dst.format);
      im_rect src_rect = {0, 0, 1920, 1080};
      //g_print("imcrop %d",src_rect.width);
      imcrop(dst,dst_output,src_rect);

      imresize(dst_output,dst_resize_output);

      data->data = this->m_data->dst_resize_output_buf;
      data->size = width*height*get_bpp_from_format(DST_FORMAT);
   }

  } else 

  if (map_info.size == 3133440 || map_info.size == 4177920 ) {
    // h264 16bit 1920 * 1080 == 1920 * 1088
    src = wrapbuffer_virtualaddr((char *) map_info.data, 1920, 1088, SRC_FORMAT);
    if (this->m_data->dst_buf == NULL){
        this->m_data->dst_buf = (char*)malloc(1920*1088*get_bpp_from_format(DST_FORMAT));
    }
    dst = wrapbuffer_virtualaddr(this->m_data->dst_buf, 1920, 1088, DST_FORMAT);
    if (this->m_data->dst_output_buf == NULL){
        this->m_data->dst_output_buf = (char*)malloc(1920*1080*get_bpp_from_format(DST_FORMAT));
    }
    dst_output = wrapbuffer_virtualaddr(this->m_data->dst_output_buf, 1920, 1080, DST_FORMAT);
    if (this->m_data->dst_resize_output_buf == NULL){
        this->m_data->dst_resize_output_buf = (char*)malloc(width*height*get_bpp_from_format(DST_FORMAT));
    }
    dst_resize_output = wrapbuffer_virtualaddr(this->m_data->dst_resize_output_buf, width, height, DST_FORMAT);
    if(src.width == 0 || dst.width == 0 || dst_output.width == 0) {
      printf("%s, %s\n", __FUNCTION__, imStrError());
      // return data;
    } else {
      // g_print("1080 h264 imcvtcolor \n");
      imcvtcolor(src, dst, src.format, dst.format);
      im_rect src_rect = {0, 0, 1920, 1080};
      //g_print("imcrop %d",src_rect.width);
      imcrop(dst,dst_output,src_rect);

      imresize(dst_output,dst_resize_output);

      data->width = width;
      data->height = height;
      data->data = this->m_data->dst_resize_output_buf;
      data->size = width*height*get_bpp_from_format(DST_FORMAT);
    }

  } else

  if (map_info.size == 6082560 || map_info.size == 8110080) {
       // h265 2560 * 1440 256 2816 1584
    src = wrapbuffer_virtualaddr((char *) map_info.data, 2816, 1440, SRC_FORMAT);
    if (this->m_data->dst_buf == NULL){
        this->m_data->dst_buf = (char*)malloc(2816*1440*get_bpp_from_format(DST_FORMAT));
    }
    dst = wrapbuffer_virtualaddr(this->m_data->dst_buf, 2816, 1440, DST_FORMAT);
    if (this->m_data->dst_output_buf == NULL){
        this->m_data->dst_output_buf = (char*)malloc(2560*1440*get_bpp_from_format(DST_FORMAT));
    }
    dst_output = wrapbuffer_virtualaddr(this->m_data->dst_output_buf, 2560, 1440, DST_FORMAT);
    if (this->m_data->dst_resize_output_buf == NULL){
        this->m_data->dst_resize_output_buf = (char*)malloc(width*height*get_bpp_from_format(DST_FORMAT));
    }
    dst_resize_output = wrapbuffer_virtualaddr(this->m_data->dst_resize_output_buf, width, height, DST_FORMAT);
    if(src.width == 0 || dst.width == 0 || dst_output.width == 0) {
      printf("%s, %s\n", __FUNCTION__, imStrError());
      // return data;
    } else {
      // g_print("1080 h264 imcvtcolor \n");
      imcvtcolor(src, dst, src.format, dst.format);
      im_rect src_rect = {0, 0, 2560, 1440};
      //g_print("imcrop %d",src_rect.width);
      imcrop(dst,dst_output,src_rect);

      imresize(dst_output,dst_resize_output);

      data->width = width;
      data->height = height;
      data->data = this->m_data->dst_resize_output_buf;
      data->size = width*height*get_bpp_from_format(DST_FORMAT);
    }
    
  } else

  if (map_info.size == 552960) {
    //h265 640*480 768 * 480
    src = wrapbuffer_virtualaddr((char *) map_info.data, 768, 480, SRC_FORMAT);
    if (this->m_data->dst_buf == NULL){
        this->m_data->dst_buf = (char*)malloc(768*480*get_bpp_from_format(DST_FORMAT));
    }
    dst = wrapbuffer_virtualaddr(this->m_data->dst_buf, 768, 480, DST_FORMAT);
    if (this->m_data->dst_output_buf == NULL){
        this->m_data->dst_output_buf = (char*)malloc(640*480*get_bpp_from_format(DST_FORMAT));
    }
    dst_output = wrapbuffer_virtualaddr(this->m_data->dst_output_buf, 640, 480, DST_FORMAT);
    if (this->m_data->dst_resize_output_buf == NULL){
        this->m_data->dst_resize_output_buf = (char*)malloc(width*height*get_bpp_from_format(DST_FORMAT));
    }
    dst_resize_output = wrapbuffer_virtualaddr(this->m_data->dst_resize_output_buf, width, height, DST_FORMAT);
    if(src.width == 0 || dst.width == 0 || dst_output.width == 0) {
      printf("%s, %s\n", __FUNCTION__, imStrError());
      // return data;
    } else {
      // g_print("1080 h264 imcvtcolor \n");
      imcvtcolor(src, dst, src.format, dst.format);
      im_rect src_rect = {0, 0, 640, 480};
      //g_print("imcrop %d",src_rect.width);
      imcrop(dst,dst_output,src_rect);

      imresize(dst_output,dst_resize_output);

      data->width = width;
      data->height = height;
      data->data = this->m_data->dst_resize_output_buf;
      data->size = width*height*get_bpp_from_format(DST_FORMAT);
    }

  }
  else {
      // h265 h264
      // supoort 1280*720 3840*2160
            
      src = wrapbuffer_virtualaddr((char *) map_info.data, source_width, source_height, SRC_FORMAT);
      if (this->m_data->dst_buf == NULL){
          this->m_data->dst_buf = (char*)malloc(source_width*source_height*get_bpp_from_format(DST_FORMAT));
      }
      dst = wrapbuffer_virtualaddr(this->m_data->dst_buf, source_width, source_height, DST_FORMAT);
      if (this->m_data->dst_output_buf == NULL){
          this->m_data->dst_output_buf = (char*)malloc(source_width*source_height*get_bpp_from_format(DST_FORMAT));
      }
      dst_output = wrapbuffer_virtualaddr(this->m_data->dst_output_buf, source_width, source_height, DST_FORMAT);
      if (this->m_data->dst_resize_output_buf == NULL){
          this->m_data->dst_resize_output_buf = (char*)malloc(width*height*get_bpp_from_format(DST_FORMAT));
      }
      dst_resize_output = wrapbuffer_virtualaddr(this->m_data->dst_resize_output_buf, width, height, DST_FORMAT);
      if(src.width == 0 || dst.width == 0 || dst_resize_output.width == 0) {
        printf("%s, %s\n", __FUNCTION__, imStrError());
        // return data;
      } else {

        imcvtcolor(src, dst, src.format, dst.format);
        imresize(dst,dst_resize_output);

        data->width = width;
        data->height = height;
        data->data = this->m_data->dst_resize_output_buf;
        data->size = width*height*get_bpp_from_format(DST_FORMAT);

      }

  }

  if (resize_width > 0 and resize_height > 0){
    // scale 640 resize data
    if (this->m_data->dst_resize_output_resize_buf == NULL){
        this->m_data->dst_resize_output_resize_buf = (char*)malloc(resize_width*resize_height*get_bpp_from_format(DST_FORMAT));
    }
    if (this->m_data->dst_output_resize_buf == NULL){
        this->m_data->dst_output_resize_buf = (char*)malloc(resize_width*resize_height*get_bpp_from_format(DST_FORMAT));
    }
    dst_two_resize_output = wrapbuffer_virtualaddr(this->m_data->dst_resize_output_resize_buf, resize_width, resize_height, DST_FORMAT);
    dst_two_output = wrapbuffer_virtualaddr(this->m_data->dst_output_resize_buf, resize_width, resize_height, DST_FORMAT);
    if(src.width == 0 || dst.width == 0 || dst_two_resize_output.width == 0) {
      printf("%s, %s\n", __FUNCTION__, imStrError());
    }else{
      imresize(dst, dst_two_resize_output, (double(resize_width)/width), (double(resize_height)/width));
      imtranslate(dst_two_resize_output, dst_two_output, 0, int((resize_height - (height * (double(resize_height)/width)))/2));
      data->data_resize = this->m_data->dst_output_resize_buf;
      data->size_resize = resize_width * resize_height * get_bpp_from_format(DST_FORMAT);
    }
  }

  gst_buffer_unmap (buf, &map_info);
  // ** 
  gst_sample_unref (samp);

  this->m_data->frame++; 

  return data;

}
