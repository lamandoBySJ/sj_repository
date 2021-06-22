#ifndef SMART_BOX_H
#define SMART_BOX_H
#include <Arduino.h>
#include "platform_debug.h"
#include <mutex>
#include <thread>
#include <functional>
#include <HTTPClient.h>
#include <Esp32httpUpdate.h>
#include <functional>
#include "AsyncMqttClientService.h" 
#include "ColorCollector.h" 
#include "app/RTC/RTC.h" 
#include "StringHelper.h" 
#include "HTTPDownload.h"
#include "LoopTaskGuard.h" 
#include "LEDIndicator.h"
#include "WiFiService.h"
#include <drivers/Timeout.h>
#include "Logger.h"

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
  ,_rtc(_mtx,Wire,13)
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
  void color_measure(MeasEventType type=MeasEventType::EventSystemMeasure);
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
    switch(wifi_event.event_id){
      case SYSTEM_EVENT_STA_STOP:
        break;
      case SYSTEM_EVENT_STA_START:
        break;
      case SYSTEM_EVENT_STA_GOT_IP:
        LEDIndicator::getLEDIndicator().io_state_wifi(true);
        _asyncMqttClientService.connect();
        TracePrinter::printTrace("MQTT Connecting...");
        break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          LEDIndicator::getLEDIndicator().io_state_wifi(false);
        break;
      case SYSTEM_EVENT_STA_CONNECTED:
        break;
      default:
        break;
    } 
  }
  void restart(){
    ESP.restart();
  }
  void timeout_measure(){
    _asyncMqttClientService.publish("SmartBox/TimeSync",_topicTimeSync);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(2));
    guard::LoopTaskGuard::getLoopTaskGuard().set_signal_id(2);
    _timeout_measure_flipper.attach(mbed::callback(this,&SmartBox::timeout_measure),std::chrono::seconds(43200));
  }
private:
  rtos::Mutex _mtx;
  rtos::Mutex _mtx_wire;
  rtos::Thread _threadCore;
  rtos::Mail<mqtt::mail_on_message_t, 64>  _mail_box_on_mqtt_message;

  RTC _rtc;
  ColorSensor _colorSensor;
  ColorCollector colorCollector;

  WiFiService wifiService;
  AsyncMqttClientService _asyncMqttClientService;
  ESPWebService espWebService;

  HTTPDownload _httpDownload; 

  std::set<String>  _topics;
  vector<String> _splitTopics;
  std::map<String,RequestType> str_map_type;
  
  mbed::Timeout _boot_flipper;
  mbed::Timeout _timeout_flipper;
  mbed::Timeout _timeout_measure_flipper;
  String _topicTimeSync;
};

#endif