#ifndef SMART_BOX_H
#define SMART_BOX_H
#include "arduino.h"
#include "platform_debug.h"
#include <mutex>
#include <thread>
#include <functional>
#include <HTTPClient.h>
#include <Esp32httpUpdate.h>
#include <functional>
#include "NetworkService.h" 
#include "ColorCollector.h" 
#include "app/RTC/RTC.h" 
#include "StringHelper.h" 
#include "HTTPDownload.h"
#include "LoopTaskGuard.h" 
#include "LEDIndicator.h"
#include "WiFiService.h"
#include "Countdown.h"

using namespace std;


enum class RequestType : uint8_t
{
  SYSTEM_MEASURE = 1,
  SERVER_MEASURE,
  MANUAL_REQUEST,
  OTA_CANCEL,
  FILE_DOWNLOAD,
  FILE_DELETE,
  ESP_RESTART
};

class  SmartBox
{
public:
  SmartBox():_mtx(),_mtx_wire()
  ,_rtc(_mtx_wire,Wire,13)
  ,_colorSensor(_mtx_wire,Wire1,2)
  ,colorCollector(_rtc,_colorSensor)

  ,wifiService()
  
  {
      str_map_type[String("server_measure")]  = RequestType::SERVER_MEASURE;
      str_map_type[String("manual_request")]  = RequestType::MANUAL_REQUEST;
      str_map_type[String("ota_cancel")]      = RequestType::OTA_CANCEL;
      str_map_type[String("file_download")]   = RequestType::FILE_DOWNLOAD;
      str_map_type[String("file_delete")]     = RequestType::FILE_DELETE;
      str_map_type[String("esp_restart")]     = RequestType::ESP_RESTART;
  }

  ~SmartBox()=default;
  void platformio_init();
  void startup();
  void task_mqtt_service();
  void task_collection_service();
  void start_web_service();
  void start_core_task();
  void color_measure();
  void start_http_update(const String& url);
  void start_https_update(const String& url);
  void onMqttConnectCallback(bool sessionPresent);
  void onMqttMessageCallback(const String& topic,const String& payload);
  void onMqttSubscribeCallback(uint16_t packetId, uint8_t qos);
  void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);
  
  void delegateMethodOnWiFiMode()
  {
    espWebService.set_start_signal(1);
  }
  void onWiFiServiceCallback(const mail_wifi_event_t& wifi_event){
    
      if(wifi_event.mode==WIFI_STA){
          switch(wifi_event.event_id){
              case SYSTEM_EVENT_STA_STOP:
              break;
              case SYSTEM_EVENT_STA_START:
              break;
              case SYSTEM_EVENT_STA_GOT_IP:
                  networkService.connect();
                  break;
              case SYSTEM_EVENT_STA_DISCONNECTED:
                    break;
              case SYSTEM_EVENT_STA_CONNECTED:
                    break;
              default:
              break;
          } 
      }
  }
private:

  rtos::Mutex _mtx,_mtx_wire;
  RTC _rtc;
  ColorSensor _colorSensor;
  ColorCollector colorCollector;

  //Countdown _countdownTimeSync;
 // Countdown _countdown;
  WiFiService wifiService;
  NetworkService networkService;
  ESPWebService espWebService;
    
 
  rtos::Thread _threadCore;
  rtos::Mail<mqtt::mail_on_message_t, 8>  _mail_box_on_mqtt_message;
  std::set<String>  _topics;
  HTTPDownload _httpDownload;
  vector<String> _splitTopics;
  std::map<String,RequestType> str_map_type;
 
  
};

#endif