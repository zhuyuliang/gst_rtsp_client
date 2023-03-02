
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
bool isInit = false;

/* init gst_init */
extern "C" void
init(){
  if (!isInit){
    isInit = true;
    gst_init (NULL, NULL);
  }
  
}

/* create Rtsp client */
extern "C" int
createRtspClient(int id, const char* url, int conn_mode)
{
    m_mutex.lock();
    sleep(2);
    init();
    g_print("setup createRtspClient %d %s\n",id,url);
    if ( mMap.find(id) != mMap.end() ) {
      return SUCCESS;
    }
    mMap.insert(pair<int,RtspClient*>(id ,new RtspClient()));
    if (mMap.find(id)->second->enable(id, url, conn_mode)){
        m_mutex.unlock();
        return SUCCESS;
    }else{
        m_mutex.unlock();
        return FAIL;
    }
}

extern "C" int
destoryRtspClientAll()
{
  m_mutex.lock();
  for( it=mMap.begin(); it!=mMap.end(); it++){
    // cout<<it->first<<"    "<<it->second<<endl;
    if (it->second != NULL){
      it->second->disable();
      delete it->second;
      it->second = NULL;
    }
  } 
  mMap.clear();
  malloc_trim(0);
  m_mutex.unlock();
  return SUCCESS;
}

extern "C" int
destoryRtspClient(int id)
{
  m_mutex.lock();
  if (mMap.find(id) != mMap.end()){
    if (mMap.find(id)->second != NULL){
      mMap.find(id)->second->disable();
      delete mMap.find(id)->second;
      mMap.find(id)->second = NULL;
    }
    // mMap.erase(id);
    malloc_trim(0);
  }
  m_mutex.unlock();
  return SUCCESS;
}

extern "C" int
isConnect(int id)
{
  if (mMap.find(id) != mMap.end()){
      return mMap.find(id)->second->isConnect();
  }else{
      return STATUS_DISCONNECT;
  }
}

/* change url */
extern "C" int
changeURL(int id, const char* url, int conn_mode)
{
  if (mMap.find(id) != mMap.end()){
      bool ret = mMap.find(id)->second->changeURL(id, url, conn_mode);
      if(ret){
        return SUCCESS;
      }else{
        return FAIL;
      }
  }else{
      return FAIL;
  }
}

/* change url */
extern "C" int
reConnect(int id)
{
  if (mMap.find(id) != mMap.end()){
      bool ret = mMap.find(id)->second->reConnect(id);
      if(ret){
        return SUCCESS;
      }else{
        return FAIL;
      }
  }else{
      return FAIL;
  }
}

extern "C" int
mRead_Opencv(int id, int& width, int& height, int& size, char*& data)
{
  if (mMap.find(id) != mMap.end()){
    if (mMap.find(id)->second->isConnect() == STATUS_CONNECTED)
    {
      FrameData *framedata = mMap.find(id)->second->read_Opencv();
      if (framedata->size != 0) {
        data = framedata->data;
        width = framedata->width;
        height = framedata->height;
        size = framedata->size;
        free( framedata);
        return SUCCESS;
      }
      free( framedata);
    }
  }
  return FAIL;

}

//* opencv
extern "C" int
mRead_Python(int id, int& width, int& height,int& size, char* data)
{
  if (mMap.find(id) != mMap.end()){
    if (mMap.find(id)->second->isConnect() == STATUS_CONNECTED)
    {
      FrameData *framedata = mMap.find(id)->second->read_Opencv();
      if (framedata->size != 0) {
        // data = framedata->data;
        memcpy( data, framedata->data, framedata->size);
        width = framedata->width;
        height = framedata->height;
        size = framedata->size;
        free( framedata);
        return SUCCESS;
      }
      free( framedata);
    }
  }
  return FAIL;

}
