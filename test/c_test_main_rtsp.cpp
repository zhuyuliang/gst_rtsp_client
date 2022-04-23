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

// #define PNAME   1
// #define RTSPCAM 'rtsp://admin:shangqu2020@192.168.2.30:554/cam/realmonitor?channel=1&subtype=0'

// void callback()
// {   
//     // PyObject
// 	//回调函数
// 	//g_print("callback");
// }

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

    // TODO multi rtsp

    char* url1 = (char*)"rtsp://admin:shangqu2020@192.168.2.30:554/cam/realmonitor?channel=1&subtype=0";
    char* url2 = (char*)"rtsp://admin:shangqu2020@192.168.2.141:554/Streaming/Channels/1";
    char* url3 = (char*)"rtsp://admin:shangqu2020@192.168.2.64:554/Streaming/Channels/1";
    char* url4 = (char*)"rtsp://admin:admin@192.168.2.50:554/Streaming/Channels/1";
    char* url5 = (char*)"rtsp://admin:shangqu2020@192.168.2.33:554//Streaming/Channels/1";

    createRtspClient(0,url1);
    createRtspClient(1,url2);
    createRtspClient(2,url3);
    createRtspClient(3,url4);
    createRtspClient(4,url5);


    do {

        if (isConnect(0) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(0,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("*");

        } else {
            g_print("dis0*");
            // sleep_ms(500);
            // if (isConnect(0) == STATUS_DISCONNECT) {
            //     destoryRtspClient(0);
            //     sleep_ms(500);
            //     createRtspClient(0,url1);
            //     sleep_ms(500);
            // }
        }

        if (isConnect(1) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(1,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("*");

        } else {
            g_print("dis1*");
            // sleep_ms(500);
            // if (isConnect(1) == STATUS_DISCONNECT) {
            //     destoryRtspClient(1);
            //     sleep_ms(500);
            //     createRtspClient(1,url2);
            //     sleep_ms(500);
            // }
        }

        if (isConnect(2) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(2,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("*");

        } else {
            g_print("dis2*");
            // sleep_ms(500);
            // if (isConnect(2) == STATUS_DISCONNECT) {
            //     destoryRtspClient(2);
            //     sleep_ms(500);
            //     createRtspClient(2,url3);
            //     sleep_ms(500);
            // }
        }

        if (isConnect(3) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(3,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("*");

        } else {
            g_print("dis3*");
            // sleep_ms(500);
            // if (isConnect(3) == STATUS_DISCONNECT) {
            //     destoryRtspClient(3);
            //     sleep_ms(500);
            //     createRtspClient(3,url4);
            //     sleep_ms(500);
            // }
        }

        if (isConnect(4) == STATUS_CONNECTED)
        {

            unsigned char* buf = (unsigned char *)malloc(1920*1080*3);
            unsigned char* buf_resize = (unsigned char *)malloc(640*640*3);
            int status = mread(4,1920,1080,640,640,buf , 1920*1080*3, buf_resize, 640*640*3);
            free(buf);
            free(buf_resize);
            g_print("*");

        } else {
            g_print("dis4*");
            // sleep_ms(500);
            // if (isConnect(4) == STATUS_DISCONNECT) {
            //     destoryRtspClient(4);
            //     sleep_ms(500);
            //     createRtspClient(4,url5);
            //     sleep_ms(500);
            // }
        }

        

    } while (1);
    
    // TODO callback
    
    // RtspClient *client = new RtspClient();
    // RtspClient *client1 = new RtspClient();

    // //client->deleteInstance();

    // // bool isSuccess = client->enable(PNAME,RTSPCAM,&callback);
    // char* url = (char*)"rtsp://admin:shangqu2020@192.168.2.30:554/cam/realmonitor?channel=1&subtype=0";
    // bool isSuccess = client->enable(PNAME,url);

    // if (isSuccess) 
    // {
    //     cout << "sucess enable \n";
    // }

    // // bool isSuccess = client->enable(PNAME,RTSPCAM,&callback);
    // bool isSuccess1 = client1->enable(0,url);

    // if (isSuccess1) 
    // {
    //     cout << "sucess enable 1 \n";
    // }

    // // RtspClient::GetInstance()->enable(PNAME,RTSPCAM,callback);

    // do {

    //     // cout << "main sleep\n";

    //     sleep_ms(200);

    //     if (client->isConnect() == STATUS_DISCONNECT)
    //     {

    //         // client->reconnect();
    //         // client->disable();
    //         // bool isSuccess = client->enable(PNAME,RTSPCAM,&callback);

    //         // if (isSuccess) 
    //         // {
    //             cout << "sucess enable \n";
    //         // } 
    //     }else if (client->isConnect() == STATUS_CONNECTED) {
    //         FrameData *  framedata = client->read(1920,1080,640,640);
    //         delete framedata;
    //         g_print("*");
    //         // cv::Mat img(480, 640 , CV_8UC3, framedata.data);
    //         // char buf1[32] = {0};
    //         // snprintf(buf1, sizeof(buf1), "%u",sizeof(time(NULL)));
    //         // std::string str = buf1;
    //         // std::string name = str + "test.jpg";
            
    //         // cv::imwrite( name, img);
    //     }

    //     if (client1->isConnect() == STATUS_CONNECTED) {
    //         FrameData * framedata = client1->read(1920,1080,640,640);
    //         delete framedata;
    //         g_print("*");
    //         // cv::Mat img(480, 640 , CV_8UC3, framedata.data);
    //         // char buf1[32] = {0};
    //         // snprintf(buf1, sizeof(buf1), "%u",sizeof(time(NULL)));
    //         // std::string str = buf1;
    //         // std::string name = str + "test.jpg";
            
    //         // cv::imwrite( name, img);
    //     }
        
    // }while(1);
    // // sleep(20);

    // // client->disable();

    // // sleep(5);

    // // if (client->isConnect())
    // // {
    // //     cout << "connect \n";
    // // }else {
    // //     cout << "disconnect \n";
    // // }

    // // bool isSuccess1 = client->enable(PNAME,RTSPCAM,&callback);

    // // if (isSuccess1) 
    // // {
    // //     cout << "sucess enable \n";
    // // }

    // // sleep(10);

    // // client->disable();

    // // RtspClient::deleteInstance();



    cout << "hellow\n";
    return 0;
}
