#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <iconv.h>
#include <sstream>
#include <string>
#include <string.h>

using namespace std;
using namespace cv;

int main()
{
    string gsurl = "rtspsrc location=rtsp://admin:shangqu2020@192.168.2.30:554/cam/realmonitor?channel=1&subtype=0 latency=200 ! decodebin ! videoconvert ! appsink sync=false";
    VideoCapture cap = VideoCapture(gsurl,cv::CAP_GSTREAMER);
    if(!cap.isOpened())
    {
        std::cout<<"cannot open captrue..."<<std::endl;
        return 0;
    }

    int fps = cap.get(5);
    cout<<"fps:"<<fps<<endl;
    Mat frame;
    bool readreturn = false;
    while(cap.isOpened())
    {  
        readreturn = cap.read(frame);

        imshow("RTSP",frame);
        if (cvWaitKey(30) == 27) 
        {
            cout << "Esc key is pressed by user" << endl;
            break;
        }
    }

    cap.release();
    return 0;
}
