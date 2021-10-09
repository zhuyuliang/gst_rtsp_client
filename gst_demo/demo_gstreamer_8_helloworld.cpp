//
// Created by toson on 20-2-10.
//
//  10.1. 第一个Hello world程序
//  一个简易的，支持Ogg/Vorbis格式的音频播放器。

#include "cstdio"

//我们现在开始创建第一个简易的应用程序 – 一个基于命令行并支持Ogg/Vorbis格式的音频播放器。
//我们只需要使用标准的 Gstreamer的组件(components)就能够开发出这个程序。
//它通过命令行来指定播放的文件。让我们开始这段旅程:
//如在 第4章中学到的那样，
//第一件事情是要通过gst_init()函数来初始化 GStreamer库。确保程序包含了 gst/gst.h 头文件，
//这样GStreamer库中的对象和函数才能够被正确地定义。你可以通过#include <gst/gst.h> 指令来包含 gst/gst.h 头文件。
//然后你可以通过函数gst_element_factory_make ()来创建不同的元件。
//对于Ogg/Vorbis音频播放器，我们需要一个源元件从磁盘读取文件。 GStreamer 中有一个”filesrc”的元件可以胜任此事。
//其次我们需要一些东西来解析从磁盘读取的文件。 GStreamer 中有两个元件可以分别来解析Ogg/Vorbis文件。
//第一个将 Ogg 数据流解析成元数据流的元件叫”oggdemux”。第二个是 Vorbis 音频解码器，通常称为”vorbisdec”。
//由于”oggdemux”为每个元数据流动态创建衬垫，所以你得为”oggdemux”元件设置"pad- added" 的事件处理函数。
//像8.1.1部分讲 解的那样，"pad-added" 的事件处理函数可以用来将 Ogg 解码元件和 Vorbis 解码元件连接起来。
//最后，我们还需要一个音频输出元件 - “alsasink”。它会将数据传送给 ALSA 音频设备。
//万事俱备，只欠东风。我们需要把所有的元件都包含到一个容器元件中 -  GstPipeline，
//然后在这个管道中一直轮循，直到我们播放完整的歌曲。我们在第6章中 学习过如何将元件包含进容器元件，
//在5.6部分了解过元件的状 态信息。我们同样需要在管道总线上加消息处理来处理错误信息和检测流结束标志。
//现在给出我们第一个音频播放器的所有代码:
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973


#include <gst/gst.h>


/*
 * Global objects are usually a bad thing. For the purpose of this
 * example, we will use them, however.
 */


GstElement *pipeline, *source, *parser, *decoder, *conv, *sink;


static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
    GMainLoop *loop = (GMainLoop *)data;


    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print ("End-of-stream\n");
            g_main_loop_quit (loop);
            break;
        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *err;


            gst_message_parse_error (msg, &err, &debug);
            g_free (debug);


            g_print ("Error: %s\n", err->message);
            g_error_free (err);


            g_main_loop_quit (loop);
            break;
        }
        default:
            break;
    }


    return TRUE;
}


///demo_gstreamer_8_helloworld.cpp:91:15: error: ‘gst_element_get_pad’ was not declared in this scope
///     sinkpad = gst_element_get_pad (decoder, "sink");
//static void
//new_pad (GstElement *element,
//         GstPad     *pad,
//         gpointer    data)
//{
//    GstPad *sinkpad;
//    /* We can now link this pad with the audio decoder */
//    g_print ("Dynamic pad created, linking parser/decoder\n");
//
//
//    sinkpad = gst_element_get_pad (decoder, "sink");
//    gst_pad_link (pad, sinkpad);
//
//
//    gst_object_unref (sinkpad);
//}


int
main (int   argc,
      char *argv[])
{
    GMainLoop *loop;
    GstBus *bus;


    /* initialize GStreamer */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);


    /* check input arguments */
    if (argc != 2) {
        g_print ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
        return -1;
    }


    /* create elements */
    pipeline = gst_pipeline_new ("audio-player");
    source = gst_element_factory_make ("filesrc", "file-source");
    parser = gst_element_factory_make ("oggdemux", "ogg-parser");
    decoder = gst_element_factory_make ("vorbisdec", "vorbis-decoder");
    conv = gst_element_factory_make ("audioconvert", "converter");
    sink = gst_element_factory_make ("alsasink", "alsa-output");
    if (!pipeline || !source || !parser || !decoder || !conv || !sink) {
        g_print ("One element could not be created\n");
        return -1;
    }


    /* set filename property on the file source. Also add a message
     * handler. */
    g_object_set (G_OBJECT (source), "location", argv[1], NULL);


    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);


    /* put all elements in a bin */
    gst_bin_add_many (GST_BIN (pipeline),
                      source, parser, decoder, conv, sink, NULL);


    /* link together - note that we cannot link the parser and
     * decoder yet, becuse the parser uses dynamic pads. For that,
     * we set a pad-added signal handler. */
    gst_element_link (source, parser);
    gst_element_link_many (decoder, conv, sink, NULL);
//    g_signal_connect (parser, "pad-added", G_CALLBACK (new_pad), NULL);


    /* Now set to playing and iterate. */
    g_print ("Setting to PLAYING\n");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_print ("Running\n");
    g_main_loop_run (loop);


    /* clean up nicely */
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);
    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));


    return 0;
}



