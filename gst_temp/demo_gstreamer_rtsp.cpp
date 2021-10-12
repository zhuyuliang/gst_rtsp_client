//
// Created by toson on 20-2-10.
//
//  gstreamer如何接入RTSP流（IP摄像头）的代码范例。
//————————————————
//版权声明：本文为CSDN博主「柳鲲鹏」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/quantum7/article/details/82151637

#include "cstdio"

#define RTSPCAM "rtsp://admin:shangqu2020@192.168.2.30:554/cam/realmonitor?channel=1&subtype=0"

#include "gst/gst.h"

static void on_pad_added(GstElement *element, GstPad *pad, gpointer data) {
    GstPad *sinkpad;
    GstElement *decoder = (GstElement *) data;
    /* We can now link this pad with the rtsp-decoder sink pad */
    g_print("Dynamic pad created, linking source/demuxer\n");
    sinkpad = gst_element_get_static_pad(decoder, "sink");
    gst_pad_link(pad, sinkpad);
    gst_object_unref(sinkpad);
}

static void cb_new_rtspsrc_pad(GstElement *element, GstPad *pad, gpointer data) {
    gchar *name;
    GstCaps *p_caps;
    gchar *description;
    GstElement *p_rtph264depay;

    name = gst_pad_get_name(pad);
    g_print("A new pad %s was created\n", name);

    // here, you would setup a new pad link for the newly created pad
    // sooo, now find that rtph264depay is needed and link them?
    p_caps = gst_pad_get_pad_template_caps(pad);

    description = gst_caps_to_string(p_caps);
    printf("%s \n", p_caps, ", %s", description, "\n");
    g_free(description);

    p_rtph264depay = GST_ELEMENT(data);

    // try to link the pads then ...
    if (!gst_element_link_pads(element, name, p_rtph264depay, "sink")) {
        printf("Failed to link elements 3\n");
    }

    g_free(name);
}


int main(int argc, char *argv[]) {
    GstElement *pipeline = NULL, *source = NULL, *rtppay = NULL, *parse = NULL,
            *decodebin = NULL, *sink = NULL, *filter1 = NULL;
    GstCaps *filtercaps = NULL;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Build Pipeline */
    pipeline = gst_pipeline_new("Toson");

    source    = gst_element_factory_make ( "rtspsrc", "source");
    g_object_set (G_OBJECT (source), "latency", 2000, NULL);
    rtppay    = gst_element_factory_make ( "rtph264depay", "depayl");
    parse     = gst_element_factory_make ( "h264parse",    "parse");
    decodebin = gst_element_factory_make ( "avdec_h264",  "decode");
#ifdef PLATFORM_TEGRA
    sink      = gst_element_factory_make ( "nveglglessink", "sink");
#else
    sink      = gst_element_factory_make ( "autovideosink", "sink");
#endif

    if (!pipeline || !source || !rtppay || !parse || !decodebin || !sink) {
        g_printerr("One element could not be created.\n");
    }

    g_object_set (G_OBJECT (sink), "sync", FALSE, NULL);

    //create_uri(url,url_size, ip_address, port);
    g_object_set(GST_OBJECT(source), "location", RTSPCAM, NULL);
    //"rtsp://<ip>:554/live/ch00_0"

    //无必要
    filter1    = gst_element_factory_make("capsfilter", "filter");
    filtercaps = gst_caps_from_string("application/x-rtp");
    g_object_set (G_OBJECT (filter1), "caps", filtercaps, NULL);
    gst_caps_unref(filtercaps);

    gst_bin_add (GST_BIN (pipeline), source);
    gst_bin_add_many (GST_BIN (pipeline), rtppay, NULL);
    // listen for newly created pads
    g_signal_connect(source, "pad-added", G_CALLBACK(cb_new_rtspsrc_pad), rtppay);
    gst_bin_add_many (GST_BIN (pipeline), parse,NULL);
    if (!gst_element_link(rtppay, parse))
    {
        printf("\nNOPE\n");
    }

    gst_bin_add_many (GST_BIN (pipeline), decodebin, sink, NULL);

    if (!gst_element_link_many(parse, decodebin, sink, NULL))
    {
        printf("\nFailed to link parse to sink");
    }

    //不是必须
    //g_signal_connect(rtppay, "pad-added", G_CALLBACK(on_pad_added), parse);

    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    GstBus* bus = gst_element_get_bus(pipeline);
    GstMessage* msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg != NULL)
    {
        gst_message_unref(msg);
    }
    gst_object_unref (bus);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

}
