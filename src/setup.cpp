
#include <string.h>
#include <map>
#include <utility>

#include <gst_rtsp_client.h>

using namespace std;

#define FAIL 0;
#define SUCCESS 1;

map<int,RtspClient*> mMap; 
map<int,RtspClient*> ::iterator it;

extern "C" int
createRtspClient (int id, const char * url,int urllen)
{
    //&& id != nullptr && url != nullptr
    if ( mMap.find(id) == mMap.end() )
    {
      mMap.insert(pair<int,RtspClient*>(id ,new RtspClient()));
      if (mMap.find(id)->second->enable(id, url, urllen)){
          return SUCCESS;
      }else{
          mMap.find(id)->second->disable();
          mMap.erase(id);
          return FAIL;
      }
    } else {
      return FAIL;
    }
}

extern "C" int
destoryRtspClientAll()
{
  for( it=mMap.begin(); it!=mMap.end(); it++){
    // cout<<it->first<<"    "<<it->second<<endl;
    it->second->disable();
  } 
  mMap.clear();
  // if (mMap.find(id) != NULL){
  //   mMap.find(id)->disable();
  //   mMap.erase(id)
  //   // rtspclient = NULL;
  // }
  return SUCCESS;
}

extern "C" int
destoryRtspClient(int id)
{
  if (mMap.find(id) != mMap.end()){
    mMap.find(id)->second->disable();
    mMap.erase(id);
    // rtspclient = NULL;
  }
  return SUCCESS;
}

// extern "C" int
// reconnectRtsp(int id)
// {
//     if (mMap.find(id) != mMap.end()){
//       mMap.find(id)->second->reconnect();
//       return SUCCESS;
//     }else{
//       return FAIL;
//     }
// }

extern "C" int
isConnect(int id)
{
  if (mMap.find(id) != mMap.end()){
      int ret = mMap.find(id)->second->isConnect();
      return ret;
  }else{
      return STATUS_DISCONNECT;
  }
}

// typedef struct _framedata_struct
// {
//     int     integer;
//     char *  c_str;
//     void *  ptr;
//     int     array[8];
// };

extern "C" int
mread(int id, int width, int height, unsigned char * buf, int *len, unsigned char * buf640, int *len640)
{
  
  if (mMap.find(id) != mMap.end()){
    if (mMap.find(id)->second->isConnect() == STATUS_CONNECTED)
    {
      FrameData framedata = mMap.find(id)->second->read(width,height);
      memcpy( buf, framedata.data, framedata.size);
      memcpy( buf640, framedata.data640, framedata.size640);
      return SUCCESS;
      // len = framedata.size;
      // g_print("len %d", len);
      // g_print("size %d", framedata.size);
      // if (sizeof(len) == framedata.size){
      //   return SUCCESS;
      // }
    }
  }
  return FAIL;

}