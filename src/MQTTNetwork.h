#ifndef MQTT_NETWORK_H
#define MQTT_NETWORK_H

#include <arduino.h>
#include <app/AsyncMqttClient/AsyncMqttClient.h>
#include <WiFiType.h>
#include <WiFi.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include <map>
#include <set>
#include <new>
#include <vector>
#include <mutex>
#include <thread>
#include "platform/mbed.h"
namespace mqtt
{
typedef struct {
    bool sessionPresent;  
} mail_subscribe_t;
typedef struct {
    uint32_t counter=0;  
    system_event_id_t id;
} mail_wifi_event_t;
typedef struct {
    uint32_t counter=0;   
    AsyncMqttClientMessageProperties properties;
    size_t index;
    size_t total;
    String topic;
    String payload;
} mail_mqtt_t;

}

using namespace mqtt;
using namespace platform_debug;

class MQTTNetwork
{
public:
    MQTTNetwork()try:_autoConnect(true),
    _threadWiFiEvent(0,osPriorityNormal,1024*2,nullptr,"thdWiFiEvent"),
    _threadOnMessage(0,osPriorityNormal,1024*2,nullptr,"thdOnMessage"),
    _threadSubscribe(0,osPriorityNormal,1024*2,nullptr,"thdSubscribe")
    {   

    }catch(const std::bad_alloc &e) {
        platform_debug::TracePrinter::printTrace(e.what());
        platform_debug::PlatformDebug::pause();
    }catch(const std::exception &e) {
        platform_debug::TracePrinter::printTrace(e.what());
        platform_debug::PlatformDebug::pause();
    }

     ~MQTTNetwork()=default;
    MQTTNetwork(const MQTTNetwork& other)=default;
    MQTTNetwork(MQTTNetwork&& other)=default;
   

    MQTTNetwork& operator = (const MQTTNetwork& that)=default;
    MQTTNetwork& operator = (MQTTNetwork&& that)=default;

    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttSubscribe(uint16_t packetId, uint8_t qos);
    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(char* topic,char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) ;
    //void onMqttMessage(const String& topic,const String& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);
    
    void init();
   
    void startup()  //throw (os::thread_error)
    {
        init();
        
        osStatus  status;
        status =  _threadWiFiEvent.start(callback(this,&MQTTNetwork::runWiFiEventService));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadWiFiEvent.get_name()):NULL);
        status =  _threadOnMessage.start(callback(this,&MQTTNetwork::run_mail_box_on_message_arrived));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnMessage.get_name()):NULL);
        status =   _threadSubscribe.start(callback(this,&MQTTNetwork::run_mail_box_topics_subscribed));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadSubscribe.get_name()):NULL);
        
        mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
        evt->id=SYSTEM_EVENT_STA_STOP;
        _mailBoxWiFiEvent.put(evt);

    }
  
    void switchWiFiMode(wifi_mode_t mode){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        WiFi.mode(mode);
    }
    void runWiFiEventService();
    void run_mail_box_on_message_arrived();
    void run_mail_box_topics_subscribed();
    bool connected();
    //void _connectToMqtt();
    //void _connectToWifi();
    // void setMqttReconnectTimer(bool start);
    // void setWifiReconnectTimer(bool start);
    // static void _thunkConnectToWifi(void* pvTimerID);
    // static void _thunkConnectToMqtt(void* pvTimerID);
    void disconnect(bool autoConnect=false);
    bool publish(const String& topic,const String& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool subscribe(const String& topic, uint8_t qos=0);
    bool unsubscribe(const String& topic);
   
    void WiFiEvent(system_event_id_t event, system_event_info_t info);
    void addOnMessageCallback(Callback<void(const String&,const String&)> func);
    void addSubscribeTopic(const String& topic,int qos=0);
    void addSubscribeTopics(std::vector<String>& vec);
    void addOnMqttConnectCallback(Callback<void(bool)> func);
    void addOnMqttDisonnectCallback(Callback<void(AsyncMqttClientDisconnectReason)> func);
    
    static MQTTNetwork* getNetworkClient(){
        static MQTTNetwork* client=nullptr;
        if(client==nullptr){
           client = new MQTTNetwork();
        }
        return client;
    }
    static void DestoryNetworkClient(){
        if(getNetworkClient()!=nullptr){
            delete getNetworkClient();
        }
    }
    
   static AsyncMqttClient* getAsyncMqttClient(){
       static AsyncMqttClient* _client=nullptr;
        if(_client==nullptr){
           _client = new AsyncMqttClient();
        }
        return _client;
   } 
    static void DestoryAsyncMqttClient(){
        if(getAsyncMqttClient()!=nullptr){
            delete getAsyncMqttClient();
        }
    }
private:

    static const char* MQTT_HOST;
    static const char* WIFI_SSID;
    static const char* WIFI_PASSWORD;
    //static IPAddress MQTT_HOST ;
    static uint16_t MQTT_PORT ;
    
    bool _autoConnect;
    rtos::Thread _threadWiFiEvent;
    rtos::Thread _threadOnMessage;
    rtos::Thread _threadSubscribe;
    
    //TimerHandle_t _mqttReconnectTimer;
   // TimerHandle_t _wifiReconnectTimer;
    rtos::Mail<mail_wifi_event_t, 8> _mailBoxWiFiEvent;
    rtos::Mail<mqtt::mail_mqtt_t, 16> _mail_box_on_message;
    rtos::Mail<mqtt::mail_subscribe_t, 2> _mail_box_on_subscribe;
    std::vector<Callback<void(const String&,const String&)>>  _onMessageCallbacks;
    std::vector<Callback<void(bool)>>  _onMqttConnectCallbacks;
    std::vector<Callback<void(AsyncMqttClientDisconnectReason)>>  _onMqttDisconnectCallbacks;
    std::set<String>  _topics;
    rtos::Mutex _mtx;
};

#endif
/*
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
      Serial.println("Publish received.");
      Serial.print("  topic: ");
      Serial.println(topic);
      Serial.print("  qos: ");
      Serial.println(properties.qos);
      Serial.print("  dup: ");
      Serial.println(properties.dup);
      Serial.print("  retain: ");
      Serial.println(properties.retain);
      Serial.print("  len: ");
      Serial.println(len);
      Serial.print("  index: ");
      Serial.println(index);
      Serial.print("  total: ");
      Serial.println(total);
    }*/
/*
      _mapWiFiEvent[SYSTEM_EVENT_STA_GOT_IP]="SYSTEM_EVENT_STA_GOT_IP";
      _mapWiFiEvent[SYSTEM_EVENT_STA_DISCONNECTED]="SYSTEM_EVENT_STA_DISCONNECTED";
      _mapMqttEvent[MqttEvent::onMqttConnect]="onMqttConnect";
      _mapMqttEvent[MqttEvent::onMqttDisconnect]="onMqttDisconnect";
      _mapMqttEvent[MqttEvent::onMqttDisconnect]="onMqttDisconnect";
      _mapMqttEvent[MqttEvent::onMqttSubscribe]="onMqttSubscribe";
      _mapMqttEvent[MqttEvent::onMqttUnsubscribe]="onMqttUnsubscribe";
      _mapMqttEvent[MqttEvent::onMqttMessage]=" onMqttMessage";
      _mapMqttEvent[MqttEvent::onMqttPublish]=" onMqttPublish";
      */