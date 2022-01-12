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
#include "LED.h"
#include "WiFiService.h"
#include <drivers/Timeout.h>
#include "Logger.h"
#include <atomic>
#include "ApplicationListener.h"
#include "Countdown.h"
#include "platformio_api.h"

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

class SyncTimeCountdown : public TimeoutCountdown
{
public:
  SyncTimeCountdown():TimeoutCountdown(43200){}
  void init(){
    
  }
  void onTimeout() override{
      AsyncMqttClientService::publish("SmartBox/TimeSync",TimeSyncBuffer::Payload());
      guard::LoopTaskGuard::getLoopTaskGuard().set_signal_id(2);
      TimeoutManager::countdown(this);
  }
private: 
};

class  SmartBox : public ApplicationListener,public TimeoutCountdown
{
public:
  SmartBox():TimeoutCountdown(30,"smb"),
  _mtx()
  ,_colorSensor(_mtx,Wire1,2)
  ,colorCollector(_colorSensor)
  ,_time_sync_epoch(0)
  {
      str_map_type[String("server_measure")]  = RequestType::SERVER_MEASURE;
      str_map_type[String("manual_request")]  = RequestType::MANUAL_REQUEST;
      str_map_type[String("ota_cancel")]      = RequestType::OTA_CANCEL;
      str_map_type[String("file_download")]   = RequestType::FILE_DOWNLOAD;
      str_map_type[String("file_delete")]     = RequestType::FILE_DELETE;
      str_map_type[String("esp_restart")]     = RequestType::ESP_RESTART;
  }

  ~SmartBox()=default;
  const char* listener() override
  {
    return "smb";
  }
  void onTimeout() override{
    ESP.restart();
  }
  void platformio_init();
  void startup();

  void start_web_service();
  void color_measure(MeasEventType type=MeasEventType::EventSystemMeasure);
  void start_http_update(const String& url);
  void start_https_update(const String& url);

  void delegateMethodOnWiFiMode()
  {
    espWebService.set_start_signal(1);
  }

  void restart(){
    ESP.restart();
  }
  /*
  void timeout_measure(){
    AsyncMqttClientService::publish("SmartBox/TimeSync",_payloadTimeSync);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(2));
    guard::LoopTaskGuard::getLoopTaskGuard().set_signal_id(2);
    TimeoutManager::countdown();
    _timeout_measure_flipper.attach(mbed::callback(this,&SmartBox::timeout_measure),std::chrono::seconds(43200));
  }*/
  
  void system_clock_time_sync(){
      _time_sync_epoch+=60;
      SystemClock::SyncTime(_time_sync_epoch);
      _timeout_system_clock_time_sync.attach(mbed::callback(this,&SmartBox::system_clock_time_sync),std::chrono::seconds(60));
  }

 /***************************
  * MQTT Event Implement
  ****************************/
  void onMqttConnect(bool sessionPresent)override{
      TracePrinter::println("GW:  onMqttConnect()\n");
      TimeoutManager::remove(this);
      LED::io_state(LedName::MQTT,true);
      for(auto topic:_topics){
        TracePrinter::printTrace("subscribe topic:"+topic);
        AsyncMqttClientService::subscribe(topic);
      }
  }
  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)override{
      TracePrinter::println("GW:  onMqttDisconnect\n");
  }
  void onMqttSubscribe(uint16_t packetId, uint8_t qos)override{
    TracePrinter::println("GW:  onMqttSubscribe\n");
    if(packetId == _topics.size()){
        AsyncMqttClientService::publish("SmartBox/TimeSync",TimeSyncBuffer::Payload());
    }
  }
  void onMqttUnsubscribe(uint16_t packetId)override{
    TracePrinter::println("GW:  onMqttUnsubscribe\n");
  }
  void onMqttMessage(const char* topic,const char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)override{
        DeserializationError error = deserializeJson(doc, payload); 
        if (error){
            return;
        }
        StringHelper::split(_splitTopics,topic,"/");
        if(_splitTopics.size()!=2){
            return;
        }

        if(_splitTopics[1]=="ServerTime"){
              if (doc.containsKey("unix_timestamp")) {
                _time_sync_epoch = doc["unix_timestamp"].as<uint32_t>();
                _timeout_system_clock_time_sync.attach(mbed::callback(this,&SmartBox::system_clock_time_sync),std::chrono::seconds(60));
                if( (millis() - time_sync_interval) > 3000){
                    ClockTimerRecorder::SyncTime(_time_sync_epoch);
                    SystemClock::SyncTime(_time_sync_epoch);
                    RTC::SyncTime(_time_sync_epoch);
                }
              }

              if( guard::LoopTaskGuard::getLoopTaskGuard().get_thread_state()!=Thread::Running){
                  guard::LoopTaskGuard::getLoopTaskGuard().loop_start();        
                  TimeoutManager::countdown(&_syncTime);
                  TimeoutManager::countdown(&ClockTimerRecorder::GetClockTimer());
              }
              time_sync_interval=millis();
              

        }else if(_splitTopics[1]=="ServerReq"){
              if (doc.containsKey("url")) {
                AsyncMqttClientService::publish("upgrade/status/"+Device::WiFiMacAddress(),String("{\"status\":\"ESP_OTA_BEGIN\"}"));
                ThisThread::sleep_for(2000);
                AsyncMqttClientService::disconnect();

                RestartCountdown* ct=new RestartCountdown(120);
                TimeoutManager::countdown(ct);
         
                start_http_update(doc["url"].as<String>());
              }else if( doc.containsKey("event_type") ){
                  switch( str_map_type[doc["event_type"].as<String>()] ){
                        case RequestType::SYSTEM_MEASURE:
                        colorCollector.post_mail_measure(MeasEventType::EventSystemMeasure,nullptr);
                        break;
                       case RequestType::SERVER_MEASURE:
                        colorCollector.post_mail_measure(MeasEventType::EventServerMeasure,nullptr);
                        break;
                        case RequestType::MANUAL_REQUEST:
                        colorCollector.post_mail_measure(MeasEventType::EventManulRequest,nullptr);
                        break;
                        case RequestType::OTA_CANCEL:
                        break;
                        case RequestType::FILE_DOWNLOAD:
                        {
                                  if (doc.containsKey("file_url")) {
                                    String path = doc["file_url"].as<String>()+String("/");
                                    if (doc.containsKey("file_list")) {
                                        JsonArray filelist=doc["file_list"].as<JsonArray>();
                                        for(String filename :  filelist){
                                            _httpDownload.execute(path+filename,filename);
                                            ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(500));
                                        }                          
                                    }                  
                                  }
                        }
                        break;
                        case RequestType::FILE_DELETE:
                        {
                                  if (doc.containsKey("file_list")) {
                                      JsonArray filelist=doc["file_list"].as<JsonArray>();
                                      for(String filename :  filelist){
                                        if(FatHelper.exists(filename) ){
                                          FatHelper.deleteFile(FFat,filename.c_str());
                                        }
                                      }
                                  }
                                  
                        }
                        break;
                        case RequestType::ESP_RESTART:
                        ESP.restart();
                        break;
                        default:break;
              }                  
          }
      }
  }
  void onMqttPublish(uint16_t packetId)override{
      TracePrinter::println("GW:  onMqttPublish\n");
  }

  void onMqttSubscribeError(const char* topic)override{
      TracePrinter::printf("GW:  onMqttSubscribeError:%s\n",topic);
  }
  void onMqttUnsubscribeError(const char* topic)override{
      TracePrinter::printf("GW:  onMqttUnsubscribeError:%s\n",topic);
  }
  void onMqttPublishError(const char* topic, uint8_t qos, bool retain, const char* payload, size_t length, bool dup, uint16_t message_id)override{
       TracePrinter::println("GW:  onMqttUnsubscribeError\n");
       TracePrinter::printf("GW:topic:  %s\n", topic);
       TracePrinter::printf("GW:payload:  %s\n", payload);
  }
  void onMqttDisconnectError(AsyncMqttClientDisconnectReason reason)override{
      TracePrinter::println("GW:  onMqttDisconnectError\n");
      TracePrinter::printf("GW:reason:  %d\n", (int)reason);
  }

  void onMqttConnectionLost()override{
      TracePrinter::println("GW:  onMqttConnectionLost\n");
      LED::io_state(LedName::MQTT,false);
      std::lock_guard<rtos::Mutex> lck(_mtxNetwork);
      if(!espWebService.isRunning()){
        ThisThread::sleep_for(1000);
        if(WiFiService::connected()){
            AsyncMqttClientService::connect();
            TimeoutManager::countdown(this);
        }
      }
  }
 /****************************
  * WiFi Event Implement
  ***************************/
  void onWiFiEvent(const system_event_id_t& wifi_event) override{
      
      if(wifi_event == SYSTEM_EVENT_STA_GOT_IP){
        std::lock_guard<rtos::Mutex> lck(_mtxNetwork);
        if(!espWebService.isRunning()){
          TracePrinter::printTrace(String("GW:")+User::getProperties().host+":"+String(User::getProperties().port,DEC));     
         // TimeoutManager::countdown(this);
          ThisThread::sleep_for(1000);
          AsyncMqttClientService::connect();
          LED::io_state(LedName::WiFi,true);
        }
        
      }else{
        LED::io_state(LedName::WiFi,false);
      }
  }

private:
  rtos::Mutex _mtx;
  rtos::Mutex _mtxNetwork;
  rtos::Thread _threadCore;

  ColorSensor _colorSensor;
  ColorCollector colorCollector;


  ESPWebService espWebService;

  HTTPDownload _httpDownload; 

  std::set<String>  _topics;
  vector<String> _splitTopics;
  std::map<String,RequestType> str_map_type;
  
  SyncTimeCountdown _syncTime;
  mbed::Timeout _timeout_system_clock_time_sync;

  std::atomic<long> _time_sync_epoch;
  unsigned long time_sync_interval=0;
  StaticJsonDocument<8192>  doc;
};

#endif