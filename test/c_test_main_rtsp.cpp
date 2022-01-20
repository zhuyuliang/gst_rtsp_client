#include <iostream>
#include <../include/gst_rtsp_client.h>
#include <string>
#include <time.h>
#include <sys/select.h>

// opencv2
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

// #include <python3.7m/Python.h>

using namespace std;

#define PNAME   1
#define RTSPCAM "rtsp://admin:shangqu2020@192.168.2.30:554/cam/realmonitor?channel=1&subtype=0"

void callback()
{   
    // PyObject
	//回调函数
	//g_print("callback");
}

static void sleep_ms(unsigned int secs)

{

    struct timeval tval;

    tval.tv_sec=secs/1000;

    tval.tv_usec=(secs*1000)%1000000;

    select(0,NULL,NULL,NULL,&tval);

}


int main()
{
    
    RtspClient *client = new RtspClient();

    RtspClient *client1 = new RtspClient();

    //client->deleteInstance();

    // bool isSuccess = client->enable(PNAME,RTSPCAM,&callback);
    bool isSuccess = client->enable(PNAME,RTSPCAM,sizeof(RTSPCAM));

    if (isSuccess) 
    {
        cout << "sucess enable \n";
    }

    // bool isSuccess = client->enable(PNAME,RTSPCAM,&callback);
    bool isSuccess1 = client1->enable(0,RTSPCAM,sizeof(RTSPCAM));

    if (isSuccess1) 
    {
        cout << "sucess enable 1 \n";
    }

    // RtspClient::GetInstance()->enable(PNAME,RTSPCAM,callback);

    do {

        // cout << "main sleep\n";

        sleep_ms(200);

        if (client->isConnect() == STATUS_DISCONNECT)
        {

            // client->reconnect();
            // client->disable();
            // bool isSuccess = client->enable(PNAME,RTSPCAM,&callback);

            // if (isSuccess) 
            // {
            //     cout << "sucess enable \n";
            // } 
        }else if (client->isConnect() == STATUS_CONNECTED) {
            FrameData *  framedata = client->read(1920,1080,640,640);
            delete framedata;
            g_print("*");
            // cv::Mat img(480, 640 , CV_8UC3, framedata.data);
            // char buf1[32] = {0};
            // snprintf(buf1, sizeof(buf1), "%u",sizeof(time(NULL)));
            // std::string str = buf1;
            // std::string name = str + "test.jpg";
            
            // cv::imwrite( name, img);
        }

        if (client1->isConnect() == STATUS_CONNECTED) {
            FrameData * framedata = client1->read(1920,1080,640,640);
            delete framedata;
            g_print("*");
            // cv::Mat img(480, 640 , CV_8UC3, framedata.data);
            // char buf1[32] = {0};
            // snprintf(buf1, sizeof(buf1), "%u",sizeof(time(NULL)));
            // std::string str = buf1;
            // std::string name = str + "test.jpg";
            
            // cv::imwrite( name, img);
        }
        
    }while(1);
    // sleep(20);

    // client->disable();

    // sleep(5);

    // if (client->isConnect())
    // {
    //     cout << "connect \n";
    // }else {
    //     cout << "disconnect \n";
    // }

    // bool isSuccess1 = client->enable(PNAME,RTSPCAM,&callback);

    // if (isSuccess1) 
    // {
    //     cout << "sucess enable \n";
    // }

    // sleep(10);

    // client->disable();

    // RtspClient::deleteInstance();

    cout << "hellow\n";
    return 0;
}
