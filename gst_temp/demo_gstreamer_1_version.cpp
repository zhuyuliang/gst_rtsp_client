//
// Created by toson on 20-2-10.
//

#include "cstdio"
#include <gst/gst.h>

int main(int argc, char *argv[]) {
    const gchar *nano_str;
    guint major, minor, micro, nano;

    gst_version(&major, &minor, &micro, &nano);

    if (nano == 1)
        nano_str = "(CVS)";
    else if (nano == 2)
        nano_str = "(Prerelease)";
    else
        nano_str = "";

    printf("This program is linked against GStreamer %d.%d.%d %s\n",
           major, minor, micro, nano_str);
    printf("This program is linked against GStreamer %d.%d.%d %d\n",
           GST_VERSION_MAJOR, GST_VERSION_MINOR, GST_VERSION_MICRO, GST_VERSION_NANO);


    return 0;
}
