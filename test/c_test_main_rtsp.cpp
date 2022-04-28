#include <iostream>
#include <string>
#include <time.h>
#include <sys/select.h>

// rtsp lib
#include <../include/gst_rtsp_client.h>
#include <../src/setup.cpp>

// opencv2
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

using namespace std;

// sleep for ms 
static void sleep_ms(unsigned int secs)
{
    struct timeval tval;
    tval.tv_sec=secs/1000;
    tval.tv_usec=(secs*1000)%1000000;
    select(0,NULL,NULL,NULL,&tval);
}


int main()
{

    // multi rtsp

    const char* url1 = "rtsp://admin:shangqu2020@192.168.2.30:554/Streaming/Channels/1";
    const char* url2 = "rtsp://admin:shangqu2020@192.168.2.141:554/Streaming/Channels/1";
    const char* url3 = "rtsp://admin:shangqu2020@192.168.2.64:554/Streaming/Channels/1";
    const char* url4 = "rtsp://admin:shangqu2020@192.168.2.50:554/Streaming/Channels/1";
    const char* url5 = "rtsp://admin:shangqu2020@192.168.2.33:554/Streaming/Channels/1";

    createRtspClient(0, url1, UDP_CONN_MODE);
    createRtspClient(1, url2, DEFAULT_CONN_MODE);
    createRtspClient(2, url3, TCP_CONN_MODE);
    createRtspClient(3, url4, TCP_CONN_MODE);
    createRtspClient(4, url5, TCP_CONN_MODE);

    do {

        if (isConnect(0) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(0,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("id:0 connected\n");

        } else {
            g_print("id:0 disconnect\n");
            sleep_ms(500);
            if (isConnect(0) == STATUS_DISCONNECT) {
                 // destoryRtspClient(0);
                 sleep_ms(500);
                 createRtspClient(0,url1, TCP_CONN_MODE);
                 sleep_ms(500);
            }
        }

        if (isConnect(1) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(1,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("id:1 connected\n");

        } else {
            g_print("id:1 disconnect\n");
            sleep_ms(500);
            if (isConnect(1) == STATUS_DISCONNECT) {
                 //destoryRtspClient(1);
                 sleep_ms(500);
                 createRtspClient(1,url2, TCP_CONN_MODE);
                 sleep_ms(500);
            }
        }

        if (isConnect(2) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(2,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("id:2 connected\n");

        } else {
            g_print("id:2 disconnect\n");
            sleep_ms(500);
            if (isConnect(2) == STATUS_DISCONNECT) {
                 //destoryRtspClient(2);
                 sleep_ms(500);
                 createRtspClient(2,url3, TCP_CONN_MODE);
                 sleep_ms(500);
            }
        }

        if (isConnect(3) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(3,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("id:3 connected\n");

        } else {
            g_print("id:3 disconnect\n");
            sleep_ms(500);
            if (isConnect(3) == STATUS_DISCONNECT) {
                //destoryRtspClient(3);
                sleep_ms(500);
                createRtspClient(3,url4, TCP_CONN_MODE);
                sleep_ms(500);
            }
        }

        if (isConnect(4) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(4,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("id:4 connected\n");

        } else {
            g_print("id:4 disconnect\n");
            sleep_ms(500);
            if (isConnect(4) == STATUS_DISCONNECT) {
                 //destoryRtspClient(4);
                 sleep_ms(500);
                 createRtspClient(4,url5, TCP_CONN_MODE);
                 sleep_ms(500);
            }
        }


    } while (1);
    
    //         // cv::Mat img(480, 640 , CV_8UC3, framedata.data);
    //         // char buf1[32] = {0};
    //         // snprintf(buf1, sizeof(buf1), "%u",sizeof(time(NULL)));
    //         // std::string str = buf1;
    //         // std::string name = str + "test.jpg";

    //         // cv::imwrite( name, img);
    
    cout << "hellow\n";
    return 0;
}
