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
    const char* urlerror = "rtsp://admin:shangqu20201@192.168.2.29:554/cam/realmonitor?channel=1&subtype=0";

    const char* url1 = "rtsp://admin:shangqu2020@192.168.2.29:554/cam/realmonitor?channel=1&subtype=0";
    // const char* url2 = "rtsp://admin:shangqu2020@192.168.2.24:554/cam/realmonitor?channel=1&subtype=0";
    // const char* url3 = "rtsp://admin:shangqu2020@192.168.2.8:554/Streaming/Channels/101";
    // const char* url4 = "rtsp://admin:shangqu2020@192.168.2.27:554/cam/realmonitor?channel=1&subtype=0";
    // const char* url5 = "rtsp://admin:shangqu2020@192.168.2.29:554/cam/realmonitor?channel=1&subtype=0";
    // const char* url6 = "rtsp://admin:shangqu2020@192.168.2.8:554/Streaming/Channels/201";
    // const char* url7 = "rtsp://admin:shangqu2020@192.168.2.33:554/Streaming/Channels/1";
    // const char* url8 = "rtsp://admin:shangqu2020@192.168.2.39:554/Streaming/Channels/1";

    init();

    createRtspClient(0, url1, TCP_CONN_MODE);
    // createRtspClient(0, urlerror, TCP_CONN_MODE);
    // createRtspClient(1, url2, TCP_CONN_MODE);
    // createRtspClient(2, url3, TCP_CONN_MODE);
    // createRtspClient(3, url4, TCP_CONN_MODE);
    // createRtspClient(4, url5, TCP_CONN_MODE);
    // createRtspClient(5, url6, TCP_CONN_MODE);
    // createRtspClient(6, url7, TCP_CONN_MODE);
    // createRtspClient(7, url8, TCP_CONN_MODE);
    int i = 0;
    int status = 0;

    do {

        if (isConnect(0) == STATUS_CONNECTED)
        {
            //rga
            // unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            // unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            // status = mRead_Rga(0,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            // free(buf);
            // free(buf_resize);
            sleep_ms(500);
            g_print("id:0 connected\n");

            //opencv
            int width;
            int height;
            int size;
            char* data;
            status = mRead_Opencv(0,width,height,size,data);
            if (status == 1){
                cv::Mat img(height,width, CV_8UC3, data);
                std::string name = "test.jpg";
                cv::imwrite( name, img);
            }

        } else {
            // g_print("id:0 disconnect %d \n", isConnect(0));
            // sleep_ms(500);
            // // if (i == 0){
            // //     g_print("changeURL******************** %d \n", isConnect(0));
            // //     i = i+1;
            // //     sleep_ms(500);
            // //     changeURL(0,url1,TCP_CONN_MODE);
            // // } else
            // if (isConnect(0) == STATUS_DISCONNECT) {
            //      g_print("reConnect******************** %d \n", isConnect(0));
            //      sleep_ms(500);
            //      reConnect(0);
            // }
            g_print("id:0 disconnect %d \n", isConnect(0));
            sleep_ms(500);
            if (isConnect(0) == STATUS_DISCONNECT) {
                 sleep_ms(500);
                 reConnect(0);
            }
        }

        // if (isConnect(1) == STATUS_CONNECTED)
        // {
        //     //rga
        //     // unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
        //     // unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
        //     // status = mRead_Rga(1,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
        //     // free(buf);
        //     // free(buf_resize);
        //     g_print("id:1 connected\n");

        //     //opencv
        //     int width;
        //     int height;
        //     int size;
        //     char* data;
        //     status = mRead_Opencv(1,width,height,size,data);
        //     if (status == 1){
        //         cv::Mat yuvNV12(height * 3 / 2,width, CV_8UC1, data);
        //         cv::Mat rgb24(height, width , CV_8UC3);
        //         cv::cvtColor(yuvNV12, rgb24, cv::COLOR_YUV2BGR_NV12);
        //         cv::Mat croprgb(1080,1920,CV_8UC3);
        //         croprgb = rgb24(cv::Rect(0,0,1920,1080)); // 裁剪后的图
        //     }

        // } else {
        //     g_print("id:1 disconnect %d \n", isConnect(1));
        //     sleep_ms(500);
        //     if (isConnect(1) == STATUS_DISCONNECT) {
        //          sleep_ms(500);
        //          reConnect(1);
        //     }
        // }

        // if (isConnect(2) == STATUS_CONNECTED)
        // {
        //     //rga
        //     // unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
        //     // unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
        //     // status = mRead_Rga(2,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
        //     // free(buf);
        //     // free(buf_resize);
        //     g_print("id:2 connected\n");

        //     //opencv
        //     int width;
        //     int height;
        //     int size;
        //     char* data;
        //     status = mRead_Opencv(2,width,height,size,data);
        //     if (status == 1){
        //         cv::Mat yuvNV12(height * 3 / 2,width, CV_8UC1, data);
        //         cv::Mat rgb24(height, width , CV_8UC3);
        //         cv::cvtColor(yuvNV12, rgb24, cv::COLOR_YUV2BGR_NV12);
        //         cv::Mat croprgb(1080,1920,CV_8UC3);
        //         croprgb = rgb24(cv::Rect(0,0,1920,1080)); // 裁剪后的图
        //     }

        // } else {
        //     g_print("id:2 disconnect %d \n", isConnect(2));
        //     sleep_ms(500);
        //     if (isConnect(2) == STATUS_DISCONNECT) {
        //          sleep_ms(500);
        //          reConnect(2);
        //     }
        // }

        // if (isConnect(3) == STATUS_CONNECTED)
        // {
        //     //rga
        //     // unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
        //     // unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
        //     // status = mRead_Rga(3,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
        //     // free(buf);
        //     // free(buf_resize);
        //     g_print("id:3 connected\n");

        //     //opencv
        //     int width;
        //     int height;
        //     int size;
        //     char* data;
        //     status = mRead_Opencv(3,width,height,size,data);
        //     if (status == 1){
        //         cv::Mat yuvNV12(height * 3 / 2,width, CV_8UC1, data);
        //         cv::Mat rgb24(height, width , CV_8UC3);
        //         cv::cvtColor(yuvNV12, rgb24, cv::COLOR_YUV2BGR_NV12);
        //         cv::Mat croprgb(1080,1920,CV_8UC3);
        //         croprgb = rgb24(cv::Rect(0,0,1920,1080)); // 裁剪后的图
        //     }

        // } else {
        //     g_print("id:3 disconnect %d \n", isConnect(3));
        //     sleep_ms(500);
        //     if (isConnect(3) == STATUS_DISCONNECT) {
        //          sleep_ms(500);
        //          reConnect(3);
        //     }
        // }

        //  if (isConnect(4) == STATUS_CONNECTED)
        // {
        //     //rga
        //     // unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
        //     // unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
        //     // status = mRead_Rga(4,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
        //     // free(buf);
        //     // free(buf_resize);
        //     g_print("id:4 connected\n");

        //     //opencv
        //     int width;
        //     int height;
        //     int size;
        //     char* data;
        //     status = mRead_Opencv(4,width,height,size,data);
        //     if (status == 1){
        //         cv::Mat yuvNV12(height * 3 / 2,width, CV_8UC1, data);
        //         cv::Mat rgb24(height, width , CV_8UC3);
        //         cv::cvtColor(yuvNV12, rgb24, cv::COLOR_YUV2BGR_NV12);
        //         cv::Mat croprgb(1080,1920,CV_8UC3);
        //         croprgb = rgb24(cv::Rect(0,0,1920,1080)); // 裁剪后的图
        //     }

        // } else {
        //     g_print("id:4 disconnect %d \n", isConnect(4));
        //     sleep_ms(500);
        //     if (isConnect(4) == STATUS_DISCONNECT) {
        //          sleep_ms(500);
        //          reConnect(4);
        //     }
        // }

        // if (isConnect(5) == STATUS_CONNECTED)
        // {
        //     //rga
        //     // unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
        //     // unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
        //     // status = mRead_Rga(5,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
        //     // free(buf);
        //     // free(buf_resize);
        //     g_print("id:5 connected\n");

        //     //opencv
        //     int width;
        //     int height;
        //     int size;
        //     char* data;
        //     status = mRead_Opencv(5,width,height,size,data);
        //     if (status == 1){
        //         cv::Mat yuvNV12(height * 3 / 2,width, CV_8UC1, data);
        //         cv::Mat rgb24(height, width , CV_8UC3);
        //         cv::cvtColor(yuvNV12, rgb24, cv::COLOR_YUV2BGR_NV12);
        //         cv::Mat croprgb(1080,1920,CV_8UC3);
        //         croprgb = rgb24(cv::Rect(0,0,1920,1080)); // 裁剪后的图
        //     }

        // } else {
        //     g_print("id:5 disconnect %d \n", isConnect(5));
        //     sleep_ms(500);
        //     if (isConnect(5) == STATUS_DISCONNECT) {
        //          sleep_ms(500);
        //          reConnect(5);
        //     }
        // }

        // if (isConnect(6) == STATUS_CONNECTED)
        // {
        //     //rga
        //     // unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
        //     // unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
        //     // status = mRead_Rga(6,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
        //     // free(buf);
        //     // free(buf_resize);
        //     g_print("id:6 connected\n");

        //     //opencv
        //     int width;
        //     int height;
        //     int size;
        //     char* data;
        //     status = mRead_Opencv(6,width,height,size,data);
        //     if (status == 1){
        //         cv::Mat yuvNV12(height * 3 / 2,width, CV_8UC1, data);
        //         cv::Mat rgb24(height, width , CV_8UC3);
        //         cv::cvtColor(yuvNV12, rgb24, cv::COLOR_YUV2BGR_NV12);
        //         cv::Mat croprgb(1080,1920,CV_8UC3);
        //         croprgb = rgb24(cv::Rect(0,0,1920,1080)); // 裁剪后的图
        //     }

        // } else {
        //     g_print("id:6 disconnect %d \n", isConnect(6));
        //     sleep_ms(500);
        //     if (isConnect(6) == STATUS_DISCONNECT) {
        //          sleep_ms(500);
        //          reConnect(6);
        //     }
        // }

        // if (isConnect(7) == STATUS_CONNECTED)
        // {
        //     //rga
        //     // unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
        //     // unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
        //     // status = mRead_Rga(7,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
        //     // free(buf);
        //     // free(buf_resize);
        //     g_print("id:7 connected\n");

        //     //opencv
        //     int width;
        //     int height;
        //     int size;
        //     char* data;
        //     status = mRead_Opencv(7,width,height,size,data);
        //     if (status == 1){
        //         cv::Mat yuvNV12(height * 3 / 2,width, CV_8UC1, data);
        //         cv::Mat rgb24(height, width , CV_8UC3);
        //         cv::cvtColor(yuvNV12, rgb24, cv::COLOR_YUV2BGR_NV12);
        //         cv::Mat croprgb(1080,1920,CV_8UC3);
        //         croprgb = rgb24(cv::Rect(0,0,1920,1080)); // 裁剪后的图
        //     }

        // } else {
        //     g_print("id:7 disconnect %d \n", isConnect(7));
        //     sleep_ms(500);
        //     if (isConnect(7) == STATUS_DISCONNECT) {
        //          sleep_ms(500);
        //          reConnect(7);
        //     }
        // }


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
