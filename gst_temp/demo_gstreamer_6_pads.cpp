//
// Created by toson on 20-2-10.
//
//  6.1 衬垫(Pads)

//如我们在Elements一章中看到的那样，衬垫(Pads)是元件对 外的接口。
//数据流从一个元件的源衬垫(source pad)到另一个元件的接收衬垫(sink pad)。
//衬垫的功能(capabilities)决定了一个元件所能处理的媒体类型。
//在这章的后续讲解中，我们将对衬垫的功能做更详细的说明。 (见第8.2节).

//一个衬垫的类型由2个特性决定:
//  . 它的数据导向(direction)以及它的时效性(availability)。
//
//正如我们先前提到 的，Gstreamer定义了2种衬垫的数据导向:源衬垫以及接收衬垫。
//衬垫的数据导向这个术语是从元件内部的角度给予定义的: 元件通过它们的接收衬垫接收数据，通过它们的源衬垫输出数据。
//如果通过一张图来形象地表述，接收衬垫画在元件的左侧，而源衬垫画在元件的右侧，数据从左向右流动。 [1]
//
//衬垫的时效性比衬垫的数据导向复杂得多。一个衬垫可以拥有三种类型的时效性:
//  . 永久型(always)、随机型(sometimes)、请求型(on request)。
//三种时效性的意义顾名思义: 永久型的衬垫一直会存在，
//随机型的衬垫只在某种特定的条件下才存在(会随机消失的衬垫也属于随机型)，
//请求型的衬垫只在应用程序明确发出请求时才出现。


#include "cstdio"

//8.1.1. 动态（随机）衬垫
//一些元件在其被创建时不会立刻产生所有它将用到的衬垫。
//例如在一个Ogg demuxer的元件中可能发生这种情况。这个元件将会读取Ogg流，
//每当它在Ogg流中检测到一些元数据流时(例如vorbis，theora )，它会为每个元数据流创建动态衬垫。
//同样，它也会在流终止时删除该衬垫。动态衬垫在demuxer这种元件中可以起到很大的作用。
//运行gst-inspect oggdemux只会显示出一个衬垫在元件中:
//一个名字叫作'sink'的接收衬垫，其它的衬垫都处于'休眠'中，
//你可以从衬垫模板(pad template)中的"Exists: Sometimes"的属性看到这些信息。
//衬垫会根据你所播放的Ogg文件的类型而产生，认识到这点 对于你创建一个动态管道特别重要。
//当元件通过它的随机型(sometimes)衬垫模板创建了一个随机型(sometimes)的衬垫的时侯，
//你可以通过对该元件绑定一个信号处理器(signal handler)，通过它来得知衬垫被创建。下面一段代码演示了如何这样做:
//名叫'sink'的接收衬垫，其它的衬垫都处于'休眠'中，显而易见这是衬垫”有时存在”的特性。
//衬垫会根据你所播放的Ogg文件的类型而产生，这点在你 准备创建一个动态管道时显得特别重要，
//当元件创建了一个”有时存在”的衬垫时，你可以通过对该元件触发一个信号处理器(signal handler) 来得知衬垫被创建。
//下面一段代码演示了如何这样做:
#include <gst/gst.h>

static void
cb_new_pad(GstElement *element,
           GstPad *pad,
           gpointer data) {
    gchar *name;


    name = gst_pad_get_name (pad);
    g_print("A new pad %s was created\n", name);
    g_free(name);


    /* here, you would setup a new pad link for the newly created pad */
    //[..]


}


int
main(int argc,
     char *argv[]) {
    GstElement *pipeline, *source, *demux;
    GMainLoop *loop;


    /* init */
    gst_init(&argc, &argv);


    /* create elements */
    pipeline = gst_pipeline_new("my_pipeline");
    source = gst_element_factory_make("filesrc", "source");
    g_object_set(source, "location", argv[1], NULL);
    demux = gst_element_factory_make("oggdemux", "demuxer");


    /* you would normally check that the elements were created properly */


    /* put together a pipeline */
    gst_bin_add_many(GST_BIN (pipeline), source, demux, NULL);
    gst_element_link_pads(source, "src", demux, "sink");


    /* listen for newly created pads */
    g_signal_connect (demux, "pad-added", G_CALLBACK(cb_new_pad), NULL);


    /* start the pipeline */
    gst_element_set_state(GST_ELEMENT (pipeline), GST_STATE_PLAYING);
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);


}
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973



