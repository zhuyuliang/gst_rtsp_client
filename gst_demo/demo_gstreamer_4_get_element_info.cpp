//
// Created by toson on 20-2-10.
//
//  5.4.1.通过元 件工厂得到元件的信息

#include "cstdio"

//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973

//像gst-inspect 这样的工具可以给出一个元件的概要:
//插件(plugin)的作者、
//描述性的元件名称(或者简称)、
//元件的等级（rank）
//以及元件的类别(category)。
//
//类别可以用来得到一个元件的类型，这个类型是在使用工厂元件创建该元件时做创建的。
//例如类别可以是 Codec/Decoder/Video(视频解码器)、Source/Video(视频发生器)、Sink/Video(视频输出器)。
//音频也有类似的类别。同样还存在 Codec/Demuxer和Codec/Muxer，甚至更多的类别。
//Gst-inspect将会列出当前所有的工厂对象，gst-inspect <factory-name> 将会列出特定工厂对象的所有概要信息。
#include <gst/gst.h>

const char *strs[] = {"audiotestsrc", "rtspsrc", "filesrc", "location",
                      "h264parse", "nvv4l2decoder", "avdec_h264", "nvstreammux",
                      "autovideosink", "nveglglessink",
                      "fakesrc", "identity", "fakesink",
                      "oggdemux",
                      "openh264dec", "d3dvideosink"
};

int
main(int argc,
     char *argv[]) {
    GstElementFactory *factory;

    /* init GStreamer */
    gst_init(&argc, &argv);

    for (auto str : strs) {
        g_print("------\n");
        factory = gst_element_factory_find(str);
        if (!factory) {
            g_print("You don't have the '%s' element installed!\n", str);
            continue;
        }
        /* display information */
        g_print("The '%s' element is a member of the category %s.\n\tDescription: %s\n",
                gst_plugin_feature_get_name (GST_PLUGIN_FEATURE(factory)),
                gst_element_factory_get_klass (factory),
                gst_element_factory_get_description (factory));
    }

    //你可以通过gst_registry_pool_feature_list (GST_TYPE_ELEMENT_FACTORY)得到所有在GStreamer中注册过的工厂元件。
    //gst_registry_pool_feature_list (GST_TYPE_ELEMENT_FACTORY)

    return 0;
}


//nvidia@nvidia-desktop:~/projects/gstreamer_l/build$ ./demo_4_get_element_info_
//------
//The 'audiotestsrc' element is a member of the category Source/Audio.
//	Description: Creates audio test signals of given frequency and volume
//------
//The 'rtspsrc' element is a member of the category Source/Network.
//	Description: Receive data over the network via RTSP (RFC 2326)
//------
//The 'filesrc' element is a member of the category Source/File.
//	Description: Read from arbitrary point in a file
//------
//You don't have the 'location' element installed!
//------
//The 'h264parse' element is a member of the category Codec/Parser/Converter/Video.
//	Description: Parses H.264 streams
//------
//The 'nvv4l2decoder' element is a member of the category Codec/Decoder/Video.
//	Description: Decode video streams via V4L2 API
//------
//The 'avdec_h264' element is a member of the category Codec/Decoder/Video.
//	Description: libav h264 decoder
//------
//The 'nvstreammux' element is a member of the category Generic.
//	Description: N-to-1 pipe stream multiplexing
//------
//The 'autovideosink' element is a member of the category Sink/Video.
//	Description: Wrapper video sink for automatically detected video sink
//------
//The 'nveglglessink' element is a member of the category Sink/Video.
//	Description: An EGL/GLES Video Output Sink Implementing the VideoOverlay interface
//------
//The 'fakesrc' element is a member of the category Source.
//	Description: Push empty (no data) buffers around
//------
//The 'identity' element is a member of the category Generic.
//	Description: Pass data without modification
//------
//The 'fakesink' element is a member of the category Sink.
//	Description: Black hole for data
//------
//The 'oggdemux' element is a member of the category Codec/Demuxer.
//	Description: demux ogg streams (info about ogg: http://xiph.org)
//------
//You don't have the 'openh264dec' element installed!
//------
//You don't have the 'd3dvideosink' element installed!

