//
// Created by toson on 20-2-10.
//
//  5.4.1.通过元 件工厂得到元件的信息

#include "cstdio"
//通过将一个源元件，零个或多个类过滤元件，和一个接收元件链接在一起，你可以建立起一条媒体管道。
//数据将在这些元件间流过。这是 GStreamer 中处理媒体的基本概念。
//图5-5 用3个链接的元件形象化了媒体管道。
//图5-5.形象化3个链接的元件
//
//通过链接这三个元件，我们创建了一条简单的元件链。
//元件链中源元件("element1")的输出将会是类过滤元件 ("element2")的输入。
//类过滤元件将会对数据进行某些操作，
//然后将数据输出给最终的接收元件("element3")。
//把上述过程想象成一个简单的 Ogg/Vorbis 音频解码器。
//源元件从磁盘读取文件。
//第二个元件就是Ogg/Vorbis 音频解码器
//。最终的接收元件是你的声卡，它用来播放经过解码的音频数据。
//我们将在该手册的后部分用一个简单的图来构建这个 Ogg/Vorbis 播放器。
//上述的过程用代码表示为:
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973
#include <gst/gst.h>

int
main (int   argc,
      char *argv[])
{
    GstElement *pipeline;
    GstElement *source, *filter, *sink;

    /* init */
    gst_init (&argc, &argv);

    /* create pipeline */
    pipeline = gst_pipeline_new ("my-pipeline");

    /* create elements */
    source = gst_element_factory_make ("fakesrc", "source");
    filter = gst_element_factory_make ("identity", "filter");
    sink = gst_element_factory_make ("fakesink", "sink");

    /* must add elements to pipeline before linking them */
    gst_bin_add_many (GST_BIN (pipeline), source, filter, sink, NULL);

    /* link */
    if (!gst_element_link_many (source, filter, sink, NULL)) {
        g_warning ("Failed to link elements!");
    }

}
//对于一些特定的链接行为，可以通过函数gst_element_link () 以及 gst_element_link_pads()来实现。
//你可以使用不同的gst_pad_link_* ()函数来得到单个衬垫的引用并将它们链接起来。
//更详细的信息请参考API手册。
//
//注意:在链接不同的元件之前，你需要确保这些元件都被加在同一个箱柜中，
//因为将一个元件加载到一个箱柜中会破坏该元件已存在的一些链接关系。
//同时，你不能直接链接不在同一箱柜或管道中的元件。
//如果你想要连接处于不同层次中的元件或衬垫，你将使用到精灵衬垫(关于精灵衬垫更多的信息将在后续章节中讲到) 。



///5.6. 元件状态
//一个元件在被创建后，它不会执行任何操作。所以你需要改变元件的状态，使得它能够做某些事情。
//Gstreamer中，元件有四种状态，每种状态都有其特定的意义。
//这四种状态为:
//  . GST_STATE_NULL:   默认状态。该状态将会回收所有被该元件占用的资源。
//  . GST_STATE_READY:  准备状态。
//                      元件会得到所有所需的全局资源，这些全局资源将被通过该元件的数据流所使用。
//                      例如打开设备、分配缓存等。但在这种状态下，数据流仍未开始被处 理，
//                      所以数据流的位置信息应该自动置0。
//                      如果数据流先前被打开过，它应该被关闭，并且其位置信息、特性信息应该被重新置为初始状态。
//  . GST_STATE_PAUSED: 在这种状态下，元件已经对流开始了处理，但此刻暂停了处理。
//                      因此该状态下元件可以修改流的位置信息，读取或者处理流数据，
//                      以及一旦状态变为 PLAYING，流可以重放数据流。这种情况下，时钟是禁止运行的。
//                      总之， PAUSED 状态除了不能运行时钟外，其它与 PLAYING 状态一模一样。
//                      处于 PAUSED 状态的元件会很快变换到 PLAYING 状态。
//                      举例来说，视频或音频输出元件会等待数据的到来，并将它们压入队列。
//                      一旦状态改变，元件就会处理接收到的数据。同样，视频接收元件能够播放数据的第 一帧。
//                      (因为这并不会影响时钟)。自动加载器（Autopluggers）可以对已经加载进管道的插件进行这种状态转换。
//                      其它更多的像codecs或者 filters这种元件不需要在这个状态上做任何事情。
//  . GST_STATE_PLAYING: PLAYING 状态除了当前运行时钟外，其它与 PAUSED 状态一模一样。
//                      你可以通过函数gst_element_set_state()来改变一个元件的状态。
//                      你如果显式地改变一个元件的状态，GStreamer可能会 使它在内部经过一些中间状态。
//                      例如你将一个元件从 NULL 状态设置为 PLAYING 状态，
//                      GStreamer在其内部会使得元件经历过 READY 以及 PAUSED 状态。
//
//当处于GST_STATE_PLAYING 状态，管道会自动处理数据。
//它们不需要任何形式的迭代。 GStreamer 会开启一个新的线程来处理数据。
//GStreamer 同样可以使用 GstBus在管道线程和应用程序现成间交互信息。
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973