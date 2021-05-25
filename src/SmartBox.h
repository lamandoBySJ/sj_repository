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
#include "HTTPDownload.h"

using namespace std;


enum class RequestType : uint8_t
{
  ALS_MEASURE = 1,
  MANUAL_REQUEST,
  OTA_CANCEL,
  FILE_DOWNLOAD,
  FILE_DELETE,
  ESP_RESTART
};

class  SmartBox
{
public:
  SmartBox()
  {
      str_map_type[String("als_measure")]   = RequestType::ALS_MEASURE;
      str_map_type[String("ota_cancel")]    = RequestType::OTA_CANCEL;
      str_map_type[String("file_download")] = RequestType::FILE_DOWNLOAD;
      str_map_type[String("file_delete")]   = RequestType::FILE_DELETE;
      str_map_type[String("esp_restart")]   = RequestType::ESP_RESTART;
      str_map_type[String("manual_request")]= RequestType::MANUAL_REQUEST;
      _topics.insert("SmartBox/TimeSync");
      _topics.insert(Platform::getDeviceInfo().BoardID+"/ServerTime");
      _topics.insert(Platform::getDeviceInfo().BoardID+"/ServerReq");
  }

  ~SmartBox()=default;

  void startup();
  void task_mqtt_service();
  void task_collection_service();
  void task_web_service();
  void start_core_task();
  
  void start_http_update(const String& url);
  void start_https_update(const String& url);
  void onMqttConnect(bool sessionPresent);
  void onMessageMqttCallback(const String& topic,const String& payload);

private:
  rtos::Mail<mqtt::test_t, 6> _mail_box_test;
  rtos::Mail<mqtt::test1_t, 6> _mail_box_test1;
  rtos::Mail<mqtt::test2_t, 6> _mail_box_test2;
  MQTTNetwork mqttNetwork;
  ColorCollector colorCollector;
  ESPWebService espWebService;
    
  rtos::Mutex _mtx;
  rtos::Thread _threadCore;
  rtos::Mail<mqtt::mail_mqtt_t, 16>  _mail_box_mqtt;
  std::set<String>  _topics;
  HTTPDownload _httpDownload;
  vector<String> _splitTopics;
  std::map<String,RequestType> str_map_type;
};

#endif