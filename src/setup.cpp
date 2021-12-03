
#include <string.h>
#include <map>
#include <utility>

#include <gst_rtsp_client.h>

using namespace std;

#define FAIL 0;
#define SUCCESS 1;

map<int,RtspClient*> Map; 
map<int,RtspClient*> ::iterator it;

extern "C" int
createRtspClient (int id, const char * url,int urllen)
{
    //&& id != nullptr && url != nullptr
    if ( Map.find(id) == Map.end() )
    {
      Map.insert(pair<int,RtspClient*>(id ,new RtspClient()));
      if (Map.find(id)->second->enable(id, url, urllen)){
          return SUCCESS;
      }else{
          Map.find(id)->second->disable();
          Map.erase(id);
          return FAIL;
      }
    } else {
      return FAIL;
    }
}

extern "C" int
destoryRtspClientAll()
{
  for( it=Map.begin(); it!=Map.end(); it++){
    // cout<<it->first<<"    "<<it->second<<endl;
    it->second->disable();
  } 
  Map.clear();
  // if (Map.find(id) != NULL){
  //   Map.find(id)->disable();
  //   Map.erase(id)
  //   // rtspclient = NULL;
  // }
  return SUCCESS;
}

extern "C" int
destoryRtspClient(int id)
{
  if (Map.find(id) != Map.end()){
    Map.find(id)->second->disable();
    Map.erase(id);
    // rtspclient = NULL;
  }
  return SUCCESS;
}

// extern "C" int
// reconnectRtsp(int id)
// {
//     if (Map.find(id) != Map.end()){
//       Map.find(id)->second->reconnect();
//       return SUCCESS;
//     }else{
//       return FAIL;
//     }
// }

extern "C" int
isConnect(int id)
{
  if (Map.find(id) != Map.end()){
      int ret = Map.find(id)->second->isConnect();
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
mread(int id, int width, int height, unsigned char * buf, int *len)
{
  
  if (Map.find(id) != Map.end()){
    if (Map.find(id)->second->isConnect() == STATUS_CONNECTED)
    {
      FrameData framedata = Map.find(id)->second->read(width,height);
      memcpy( buf, framedata.data, framedata.size);
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