#ifndef __SMART_BOX_H
#define __SMART_BOX_H
#include "arduino.h"
#include "platform_debug.h"
#include <mutex>
#include <thread>
#include <functional>
#include <HTTPClient.h>
#include <Esp32httpUpdate.h>
#include <functional>
#include "platform/mbed.h"
#include "MQTTNetwork.h" 
#include "ColorCollector.h" 
#include "app/TimeMachine/TimeMachine.h" 
#include "StringHelper.h" 
#include <vector>
using namespace std;
using namespace platform_debug;

enum class RequestType : uint8_t
{
  ALS_MEASURE = 1,
  MANUAL_REQUEST,
  OTA_CANCEL,
  FILE_DOWNLOAD,
  FILE_DELETE,
  ESP_RESTART
};

class HTTPDownload
{
public:
    HTTPDownload(){

    }
    void init(){
      vfilelist.clear();
      vfileDownloaded.clear();
    }
    ~HTTPDownload()=default;

    bool execute(const String& url,const String& file)
    {
      _path =  String("/")+file; 
      _httpClient.begin(url);
      int httpCode = _httpClient.GET();
       
      if((httpCode != HTTP_CODE_OK)){
        return false;
      }
             
      bool success=true;
      if(httpCode > 0 && httpCode == HTTP_CODE_OK) {
          int len = _httpClient.getSize();
          int buff_len = sizeof(buff);
          // get tcp stream
          WiFiClient* stream = _httpClient.getStreamPtr();
          FFatHelper::writeFile(FFat,_path,"");   
          // read all data from server
          while( _httpClient.connected() && (len > 0 || len == -1)) {
              // get available data size
              size_t size = stream->available();
              platform_debug::TracePrinter::printf("[HTTP] size : %d, len: %d\n",size,len);
              if(size>0) {
                // read up to 128 byte
                if(size <= buff_len){
                    buff_len=size;
                }else{
                    buff_len=sizeof(buff);
                }
                int c = stream->readBytes(buff, buff_len);
                if(len > 0) {
                    len -= c;
                }
                if(c == buff_len){
                    FFatHelper::appendFile(FFat,_path,buff,c); 
                }else{
                    success=false;
                break;
              }
          }
        }
      } 
      if(success==true){
         vfileDownloaded.push_back(file);
      }
      _httpClient.end();
    }
private:
    uint8_t buff[1024] = { 0 };
    HTTPClient _httpClient;
    std::vector<String> vfilelist;
    std::vector<String> vfileDownloaded;
    String splitString;
    String _path;
};

class SmartBox
{
public:
  SmartBox():_thread(osPriorityNormal,1024*6),_mtx(),
    _timeMachine(_mtx,21,22,13),
    _colorCollector(),
    _webService()
  {
      str_map_type[String("als_measure")]   = RequestType::ALS_MEASURE;
      str_map_type[String("ota_cancel")]    = RequestType::OTA_CANCEL;
      str_map_type[String("file_download")] = RequestType::FILE_DOWNLOAD;
      str_map_type[String("file_delete")]   = RequestType::FILE_DELETE;
      str_map_type[String("esp_restart")]   = RequestType::ESP_RESTART;
      str_map_type[String("manual_request")]= RequestType::MANUAL_REQUEST;
  }

  ~SmartBox()=default;

  void startup();

  void run_core_task();

  void start_web_task(){
      _webService.setCallbackPostMailToCollector(callback(&_colorCollector,&ColorCollector::delegateMethodPostMail));
      _colorCollector.setCallbackWebSocketClientEvent(callback(&_webService,&ESPWebService::delegateMethodWebSocketClientEvent));
      _colorCollector.setCallbackWebSocketClientText(callback(&_webService,&ESPWebService::delegateMethodWebSocketClientText));
      _webService.startup();
  }
  void start_http_update(const String& url);
  void start_https_update(const String& url);
  void onMqttConnect(bool sessionPresent);
  void onMessageMqttCallback(const String& topic,const String& payload);

private:

  rtos::Thread _thread;
  rtos::Mutex _mtx;
  TimeMachine<DS1307,rtos::Mutex> _timeMachine;
  
  ColorCollector  _colorCollector;
  ESPWebService  _webService;
  HTTPDownload _httpDownload;
  vector<String> _splitTopics;
  std::map<String,RequestType> str_map_type;
  rtos::Mail<mqtt::mail_mqtt_t, 16>  _mail_box_mqtt;

};

#endif