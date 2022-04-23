
#include <string.h>
#include <unordered_map>
#include <utility>

#include <gst_rtsp_client.h>
#include <mutex>
#include <malloc.h>

using namespace std;

#define FAIL 0;
#define SUCCESS 1;

unordered_map<int,RtspClient*> mMap; 
unordered_map<int,RtspClient*> ::iterator it;

mutex m_mutex;

extern "C" int
createRtspClient (int id, const char * url)
{
    m_mutex.lock();
    //&& id != nullptr && url != nullptr
    g_print("setup createRtspClient %d %s\n",id,url);
    if ( mMap.find(id) == mMap.end() )
    {
      mMap.insert(pair<int,RtspClient*>(id ,new RtspClient()));
      if (mMap.find(id)->second->enable(id, url)){
          m_mutex.unlock();
          return SUCCESS;
      }
      // else{
      //     // mMap.find(id)->second->disable();
      //     // delete mMap.find(id)->second;
      //     // mMap.erase(id);
      //     m_mutex.unlock();
      //     return FAIL;
      // }
    }
    //  else {
    //   // mMap.find(id)->second->disable();
    //   // delete mMap.find(id)->second;
    //   // mMap.erase(id);
    //   return FAIL;
    // }
    m_mutex.unlock();
    return FAIL;
}

extern "C" int
createRtspClient (int id, char * url, FRtspCallBack frtspcallback)
{
    m_mutex.lock();
    //&& id != nullptr && url != nullptr
    g_print("setup createRtspClient %d \n",id);
    if ( mMap.find(id) == mMap.end() )
    {
      mMap.insert(pair<int,RtspClient*>(id ,new RtspClient()));
      if (mMap.find(id)->second->enable(id, url, frtspcallback)){
          m_mutex.unlock();
          return SUCCESS;
      }
      // else{
      //     // mMap.find(id)->second->disable();
      //     // delete mMap.find(id)->second;
      //     // mMap.erase(id);
      //     return FAIL;
      // }
    }
    //  else {
      // mMap.find(id)->second->disable();
      // delete mMap.find(id)->second;
      // mMap.erase(id);
    m_mutex.unlock();
    return FAIL;
    // }
}

extern "C" int
destoryRtspClientAll()
{
  m_mutex.lock();
  for( it=mMap.begin(); it!=mMap.end(); it++){
    // cout<<it->first<<"    "<<it->second<<endl;
    it->second->disable();
    delete it->second;
    it->second = NULL;
  } 
  mMap.clear();
  malloc_trim(0);
  // if (mMap.find(id) != NULL){
  //   mMap.find(id)->disable();
  //   mMap.erase(id)
  //   // rtspclient = NULL;
  // }
  m_mutex.unlock();
  return SUCCESS;
}

extern "C" int
destoryRtspClient(int id)
{
  m_mutex.lock();
  if (mMap.find(id) != mMap.end()){
    mMap.find(id)->second->disable();
    delete mMap.find(id)->second;
    mMap.find(id)->second = NULL;
    mMap.erase(id);
    malloc_trim(0);
    // rtspclient = NULL;
  }
  m_mutex.unlock();
  return SUCCESS;
}

extern "C" int
isConnect(int id)
{
  // m_mutex.lock();
  if (mMap.find(id) != mMap.end()){
      int ret = mMap.find(id)->second->isConnect();
      // m_mutex.unlock();
      return ret;
  }else{
      // g_print ("isConnect STATUS_DISCONNECT \n");
      // m_mutex.unlock();
      return STATUS_DISCONNECT;
  }
}

extern "C" int
mread(int id, int width, int height, int resize_width, int resize_height, unsigned char * buf, int len, unsigned char * buf_resize, int len_resize)
{
  // m_mutex.lock();
  if (mMap.find(id) != mMap.end()){
    if (mMap.find(id)->second->isConnect() == STATUS_CONNECTED)
    {
      FrameData *framedata = mMap.find(id)->second->read(width, height, resize_width, resize_height);
      if (framedata->size != 0) {
        memcpy( buf, framedata->data, framedata->size);
        if (resize_width > 0 and resize_height >0){
          memcpy( buf_resize, framedata->data_resize, framedata->size_resize);
        }
        free( framedata);
        m_mutex.unlock();
        return SUCCESS;
      }
      free( framedata);
    }
  }
  // m_mutex.unlock();
  return FAIL;

}