//
// Created by toson on 20-2-10.
//
//  简易初始化

#include "cstdio"
#include <gst/gst.h>

//你可以使用GST_VERSION_MAJOR, GST_VERSION_MINOR以及GST_VERSION_MICRO 三个宏得到你的GStreamer版本信息，
//或者使用函数gst_version得到当前你所调用的程序库的版本信息。
//目前GStreamer使用了一种 保证主要版本和次要版本中API-/以及ABI兼容的策略。
//当命令行参数不需要被GStreamer解析的时候，你可以在调用函数gst_init时使用2个NULL参数。
//注[1] 这个例子中的代码可以直接提取出来,并在GStreamer的examples/manual目录下可以找到。
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973
int main(int argc, char *argv[]) {
    guint major, minor, micro, nano;
    gst_version(&major, &minor, &micro, &nano);
    printf("GStreamer version: %d.%d.%d %s\n",
           major, minor, micro, nano == 1 ? "(CVS)" : nano == 2 ? "(Prerelease)" : "");

    gst_init(&argc, &argv);

    return 0;
}


//你同样可以使用GOption表来初始化你的参数。
//如例子中的代码所示，你可以通过 GOption 表来定义你的命令行选项。
//将表与由 gst_init_get_option_group 函数返回的选项组一同传给GLib初始化函数。
//通过使用GOption表来初始化GSreamer，你的程序还可以解析除标准GStreamer选项以外的命令行选项.
//————————————————
//版权声明：本文为CSDN博主「北雨南萍」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/fireroll/article/details/46859973
#include <gst/gst.h>

int main1(int argc, char *argv[]) {
    guint major, minor, micro, nano;
    gst_version(&major, &minor, &micro, &nano);
    printf("GStreamer version: %d.%d.%d %s\n", major, minor, micro,
           nano == 1 ? "(CVS)" : nano == 2 ? "(Prerelease)" : "");

    gboolean silent = FALSE;
    gchar *savefile = NULL;
    GOptionContext *ctx;
    GError *err = NULL;
    GOptionEntry entries[] = {
            {"silent", 's', 0, G_OPTION_ARG_NONE,   &silent,
                    "do not output status information", NULL},
            {"output", 'o', 0, G_OPTION_ARG_STRING, &savefile,
                    "save xml representation of pipeline to FILE and exit", "FILE"},
            {NULL}
    };

    ctx = g_option_context_new("- Your application");
    g_option_context_add_main_entries(ctx, entries, NULL);
    g_option_context_add_group(ctx, gst_init_get_option_group());
    if (!g_option_context_parse(ctx, &argc, &argv, &err)) {
        g_print("Failed to initialize: %s\n", err->message);
        g_error_free(err);
        return 1;
    }

    printf("Run me with --help to see the Application options appended.\n");

    return 0;
}