//
// Created by steve 21-09-16
//
//  gstreamer如何接入RTSP流（IP摄像头） h264/h265
//  uridecodebin videoconvert autovideosink 

#include "cstdio"

#include <pthread.h>

#define RTSPCAM "rtsp://admin:shangqu2020@192.168.2.30:554/cam/realmonitor?channel=1&subtype=0"

#include "gst/gst.h"


// struct CustomData {
//     GMainLoop *loop;
//     GstElement *pipeline;
//     GstElement *sink;
//     pthread_t gst_thread;
// };

static void on_pad_added(GstElement *element, GstPad *pad, gpointer data) {
    GstPad *sinkpad;
    GstElement *decoder = (GstElement *) data;
    /* We can now link this pad with the rtsp-decoder sink pad */
    g_print("Dynamic pad created, linking source/demuxer\n");
    sinkpad = gst_element_get_static_pad(decoder, "sink");
    gst_pad_link(pad, sinkpad);
    gst_object_unref(sinkpad);
}

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

static void msg_bus (GstBus *bus, GstMessage *msg, GMainLoop *pipline) {
    GError *error;
    gchar *debug_info;

    gst_message_parse_error ( msg, &error, &debug_info );
    g_printerr( " Error received from element %s: %s\n",GST_OBJECT_NAME (msg->src), error->message);
    g_printerr( "Debuging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error (&error);
    g_free (debug_info);

    g_main_loop_quit (pipline);
}


int main(int argc, char *argv[]) {

    GMainLoop *main_loop;

    GstElement *pipeline = NULL, *uridecodebin = NULL, *sink1 = NULL, *sink2 = NULL, *convert = NULL;
    GstElement *queue1 = NULL, *queue2 = NULL;
    GstElement *tee1 = NULL;

    GstCaps *video_caps = NULL;
    GstBus *bus = NULL;
    GstPad *queue1_video_pad = NULL, *queue2_video_pad = NULL;
    GstPad *tee1_video_pad = NULL, *tee2_video_pad = NULL;

    // GstElement *videoxraw = NULL;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Build Pipeline */
    pipeline = gst_pipeline_new("Uridecodebin265");

    uridecodebin = gst_element_factory_make ( "uridecodebin", "source");
    tee1 = gst_element_factory_make ( "tee", "tee1");

    queue1 = gst_element_factory_make ( "queue", "queue1");
    queue2 = gst_element_factory_make ( "queue", "queue2");

    // videoxraw = gst_element_factory_make ( "video/x-raw", "xraw");
    // convert = gst_element_factory_make("videoconvert", "convert");
#ifdef PLATFORM_TEGRA
    //sink      = gst_element_factory_make ( "nveglglessink", "sink");
#else
    // sink      = gst_element_factory_make ( "autovideosink", "sink");
    sink1      = gst_element_factory_make ( "rkximagesink", "sink1");
    sink2      = gst_element_factory_make ( "appsink", "sink2");
#endif

    if (!pipeline || !uridecodebin || !tee1 || !queue1 || !queue2  || !sink1 || !sink2 ) {
        g_printerr("One element could not be created.\n");
    }

    // Configure rksink
    g_object_set (G_OBJECT (sink1), "sync", FALSE, NULL);

    // Config appsink
    g_object_set (G_OBJECT (sink2), "sync", FALSE, NULL);
    g_object_set (G_OBJECT (sink2), "emit-signals", TRUE, NULL );
    g_signal_connect ( G_OBJECT (sink2), "new-sample", G_CALLBACK (new_sample), pipeline);


    //create_uri(url,url_size, ip_address, port);
    g_object_set(GST_OBJECT(uridecodebin), "uri", RTSPCAM, NULL);
    //"rtsp://<ip>:554/live/ch00_0"

    gst_bin_add_many(GST_BIN(pipeline), uridecodebin, tee1, NULL);
    gst_bin_add_many(GST_BIN(pipeline), queue1, sink1, NULL);
    gst_bin_add_many(GST_BIN(pipeline), queue2, sink2, NULL);

    // queue -> rkximagesink    
    if (!gst_element_link_many (queue1, sink1, NULL)) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    // queue -> appsink
    if (!gst_element_link_many (queue2, sink2, NULL)) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    //tee -> queue1 -> queue2
    queue1_video_pad = gst_element_get_static_pad ( queue1, "sink");
    queue2_video_pad = gst_element_get_static_pad ( queue2, "sink");
    tee1_video_pad = gst_element_get_request_pad ( tee1, "src_%u");
    tee2_video_pad = gst_element_get_request_pad ( tee1, "src_%u");
    if (gst_pad_link ( tee1_video_pad, queue1_video_pad) != GST_PAD_LINK_OK || gst_pad_link ( tee2_video_pad, queue2_video_pad) != GST_PAD_LINK_OK) {
        g_printerr ("tee link queue error. \n");
        gst_object_unref (pipeline);
    }
    gst_object_unref (queue1_video_pad);
    gst_object_unref (queue2_video_pad);

    //gst_element_release_request_pad (tee1,tee1_video_pad);
    //gst_element_release_request_pad (tee1,tee2_video_pad);
    gst_object_unref (tee1_video_pad);
    gst_object_unref (tee2_video_pad);
    // gst_object_unref (tee1);

    g_signal_connect(uridecodebin, "pad-added", G_CALLBACK(on_pad_added), tee1);

    // gst_object_unref (tee1);
    //gst_object_unref (uridecodebin);

    bus = gst_element_get_bus(pipeline);
    gst_bus_add_signal_watch (bus);
    g_signal_connect ( G_OBJECT(bus), "message::error", (GCallback)msg_bus, pipeline);
    gst_object_unref (bus);

    // start playing
    g_print ("start playing");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    main_loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (main_loop);
    
    // destroy play
    g_print ("destory play");

    gst_object_unref (sink1);
    gst_object_unref (sink2);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;

}
