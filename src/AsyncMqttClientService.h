#ifndef ASYNC_MQTT_CLIENT_SERVICE_H
#define ASYNC_MQTT_CLIENT_SERVICE_H

#include <arduino.h>
#include <app/AsyncMqttClient/AsyncMqttClient.h>
#include <WiFiType.h>
#include <WiFi.h>
#include <map>
#include <set>
#include <new>
#include <vector>
#include <mutex>
#include <thread>
#include <esp_event_legacy.h>
#include "platform_debug.h"
#include "platformio_api.h"

namespace mqtt
{

struct [[gnu::may_alias]]  mail_on_connect_t {
    bool sessionPresent;  
    uint32_t counter=0; 
} ;
struct [[gnu::may_alias]]  mail_on_subscribe_t {
    uint8_t qos;
    uint16_t packetId;
    uint32_t counter=0; 
} ;
struct [[gnu::may_alias]]  mail_on_unsubscribe_t {
    uint16_t packetId;
    uint32_t counter=0; 
} ;
struct [[gnu::may_alias]] mail_on_message_t {
    size_t index;
    size_t total;
    AsyncMqttClientMessageProperties properties;
    String topic;
    String payload;
};

}

class AsyncMqttClientService
{
public:

    AsyncMqttClientService()try:
    _threadOnMessage(0,osPriorityNormal,1024*2,nullptr,"thdOnMessage"),
    _threadOnConnect(0,osPriorityNormal,1024*2,nullptr,"thdOnConnect"),
    _threadOnSubscribe(0,osPriorityNormal,1024*2,nullptr,"thdOnSubscribe"),
    _threadOnUnsubscribe(0,osPriorityNormal,1024*2,nullptr,"thdOnUnsubscribe")
    {   
      

    }catch(const std::bad_alloc &e) {
        TracePrinter::printTrace(e.what());
        PlatformDebug::pause();
    }catch(const std::exception &e) {
        TracePrinter::printTrace(e.what());
        PlatformDebug::pause();
    }

     ~AsyncMqttClientService()=default;
    AsyncMqttClientService(const AsyncMqttClientService& other)=default;
    AsyncMqttClientService(AsyncMqttClientService&& other)=default;
   

    AsyncMqttClientService& operator = (const AsyncMqttClientService& that)=default;
    AsyncMqttClientService& operator = (AsyncMqttClientService&& that)=default;

    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttSubscribe(uint16_t packetId, uint8_t qos);
    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(char* topic,char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) ;
    //void onMqttMessage(const String& topic,const String& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);

    void startup();  //throw (os::thread_error)

    bool connected();
    void connect();
    void disconnect();
    bool publish(const String& topic,const String& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool subscribe(const String& topic, uint8_t qos=0);
    bool unsubscribe(const String& topic);
   
    void WiFiEvent(system_event_id_t event, system_event_info_t info);
    void addOnMqttMessageCallback(mbed::Callback<void(const String&,const String&)> func);
    void addOnMqttConnectCallback(mbed::Callback<void(bool)> func);
    void addOnMqttDisonnectCallback(mbed::Callback<void(AsyncMqttClientDisconnectReason)> func);
    void addOnMqttSubscribeCallback(mbed::Callback<void(uint16_t, uint8_t)> func);
    void addOnMqttUnsubscribeCallback(mbed::Callback<void(uint16_t)> func);
 

    void removeOnMqttMessageCallbacks(){
        _onMqttMessageCallbacks.clear();
    }
    void removeOnMqttConnectCallbacks(){
        _onMqttConnectCallbacks.clear();
    }
    void removeOnMqttDisconnectCallbacks(){
        _onMqttDisconnectCallbacks.clear();
    }
    void removeOnMqttSubscribeCallback(){
        _onMqttSubscribeCallbacks.clear();
    }
    void removeOnMqttUnsubscribeCallbacks()
    {
        _onMqttUnsubscribeCallbacks.clear();
    }
    
    void cleanupCallbacks(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
         _onMqttMessageCallbacks.clear();
         _onMqttConnectCallbacks.clear();
         _onMqttDisconnectCallbacks.clear();
        _onMqttSubscribeCallbacks.clear();
        _onMqttUnsubscribeCallbacks.clear();
    }
    
    void shutdown()
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        _threadOnMessage.terminate();
        _threadOnSubscribe.terminate();
        _threadOnUnsubscribe.terminate();
        _threadOnConnect.terminate();
    }

    void task_on_message_service();
    void task_on_connect_service();
    void task_on_subscribe_service();
    void task_on_unsubscribe_service();
    void init();
    bool delegateMethodPublish(const String& topic,const String& payload){
       return publish(topic,payload);
    }
protected:
    
private:
    
  
    rtos::Mail<mqtt::mail_on_message_t, 16> _mail_box_on_message;
    rtos::Mail<mqtt::mail_on_connect_t, 8> _mail_box_on_connect; 
    rtos::Mail<mqtt::mail_on_subscribe_t, 8> _mail_box_on_subscribe;
    rtos::Mail<mqtt::mail_on_unsubscribe_t, 8> _mail_box_on_unsubscribe;
    //static IPAddress MQTT_HOST ;
   
    rtos::Thread _threadOnMessage;
    rtos::Thread _threadOnConnect;
    rtos::Thread _threadOnSubscribe;
    rtos::Thread _threadOnUnsubscribe;
    rtos::Mutex _mtx;
    std::vector<mbed::Callback<void(const String&,const String&)>>  _onMqttMessageCallbacks;
    std::vector<mbed::Callback<void(bool)>>  _onMqttConnectCallbacks;
    std::vector<mbed::Callback<void(AsyncMqttClientDisconnectReason)>>  _onMqttDisconnectCallbacks;
    std::vector<mbed::Callback<void(uint16_t, uint8_t)>>  _onMqttSubscribeCallbacks;
    std::vector<mbed::Callback<void(uint16_t packetId)>>  _onMqttUnsubscribeCallbacks;
   
    AsyncMqttClient _client;
};

#endif