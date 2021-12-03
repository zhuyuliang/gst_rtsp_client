//
// Created by toson on 20-2-10.
//
//  5.2. 创建一个GstElement对象

#include "cstdio"

//创建一个元件的最简单的方法是通过函数gst_element_factory_make ()。
//这个函数使用一个已存在的工厂对象名和一个新的元件名来创建元件。
//创建完之后, 你可以用新的元件名在箱柜（bin）中查询得到这个元件。
//这个名字同样可以用来调试程序的输 出。你可以通过传递 NULL 来得到一个默认的具有唯一性的名字。
//当你不再需要一个元件时，你需要使用 gst_object_unref ()来对它进行解引用。
//这会将一个元件的引用数减少1。任何一个元件在创建时，其引用记数为1。当其引用记数为0时，该元件会被销毁。
//
//下面的例子[1] 显示了如果通过一个fakesrc工厂对象来创建一个名叫source的元件。
//程序会检查元件是否创建成功。检查完毕后，程序会销毁元件。
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973
#include <gst/gst.h>

int
main1 (int   argc,
      char *argv[])
{
    GstElement *element;

    /* init GStreamer */
    gst_init (&argc, &argv);

    /* create element */
    element = gst_element_factory_make ("fakesrc", "source");
    if (!element) {
        g_print ("Failed to create element of type 'fakesrc'\n");
        return -1;
    }

    gst_object_unref (GST_OBJECT (element));

    return 0;
}

//gst_element_factory_make 是2个函数的速记。
//一个GstElement 对象由工厂对象创建而来。
//为了创建一个元件，你需要使用一个唯一的工厂对象名字来访问一个  GstElementFactory对象。
//gst_element_factory_find ()就 是做了这样的事。
//
//下面的代码段创建了一个工厂对象，这个工厂对象被用来创建一个fakesrc元件 —— 伪装的数据源。
//函数 gst_element_factory_create() 将会使用元件工厂并根据给定的名字来创建一个元件。
#include <gst/gst.h>

int
main2 (int   argc,
      char *argv[])
{
    GstElementFactory *factory;
    GstElement * element;

    /* init GStreamer */
    gst_init (&argc, &argv);

    /* create element, method #2 */
    factory = gst_element_factory_find ("fakesrc");
    if (!factory) {
        g_print ("Failed to find factory of type 'fakesrc'\n");
        return -1;
    }
    element = gst_element_factory_create (factory, "source");
    if (!element) {
        g_print ("Failed to create element, even though its factory exists!\n");
        return -1;
    }

    gst_object_unref (GST_OBJECT (element));

    return 0;
}
//GstElement的属性大多通过标准的 GObject 对象实现的。
//使用 GObject 的方法可以对GstElement实行查询、设置、获取属性的值。同样 GParamSpecs 也被支持。
//每个 GstElement 都从其基类 GstObject 继承了至少一个“名字”属性。
//这个名字属性将在函数gst_element_factory_make ()或者函数gst_element_factory_create ()中使用到。
//你可通过函数 gst_object_set_name 设置该属性，通过 gst_object_get_name 得到一个对象的名字属性。
//你也可以通过下面的方法来得到一个对象的名字属性。
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973
#include <gst/gst.h>

int
main (int   argc,
      char *argv[])
{
    GstElement *element;
    gchar *name;

    /* init GStreamer */
    gst_init (&argc, &argv);

    /* create element */
    element = gst_element_factory_make ("fakesrc", "source1234324234");

    /* get name */
    g_object_get (G_OBJECT (element), "name", &name, NULL);
    g_print ("The name of the element is '%s'.\n", name);
    g_free (name);

    gst_object_unref (GST_OBJECT (element));

    return 0;
}
//大多数的插件(plugins)都提供了一些额外的方法，这些方法给程序员提供了更多的关于该元件的注册信息或配置信息。
//gst-inspect 是一个用来查询特定元件特性（properties）的实用工具。
//它也提供了诸如函数简短介绍，参数的类型及其支持的范围等信息。关于 gst-inspect 更详细的信息请参考附录。
//关于GObject特性更详细的信息，我们推荐你去阅读 GObject手册 以及  Glib 对象系统介绍.
//GstElement对象同样提供了许多的 GObject 信号方法来实现一个灵活的回调机制。
//你同样可以使用 gst-inspect来检查一个特定元件所支持的信号。
//总之，信号和特性是元件与应用程序交互的最基本的方式。
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973
