#ifndef MQTT_NETWORK_H
#define MQTT_NETWORK_H

#include <arduino.h>
#include <app/AsyncMqttClient/AsyncMqttClient.h>
#include <WiFiType.h>
#include <WiFi.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include "OTAService.h"
#include <map>
#include <set>
#include <new>
#include <vector>
#include <mutex>
#include <thread>
namespace mqtt
{
typedef struct {
    bool sessionPresent;  
} mail_on_connect_t;
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
} mail_t;

}

using namespace mqtt;
using namespace platform_debug;

class MQTTNetwork
{
public:
    MQTTNetwork():
    _threadWiFiEvent(osPriorityNormal,1024*2),
    _threadOnMessage(osPriorityNormal,1024*3),
    _threadSubscribe(osPriorityNormal,1024*2),
    _autoConnect(true)
    {
     
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
   
    void startup(){
        init();
        _threadOnMessage.start(callback(this,&MQTTNetwork::run_mail_box_on_message_arrived));
        _threadSubscribe.start(callback(this,&MQTTNetwork::run_mail_box_topics_subscribed));
        _threadWiFiEvent.start(callback(this,&MQTTNetwork::runWiFiEventService));
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
private:
    static const char* MQTT_HOST;
    static const char* WIFI_SSID;
    static const char* WIFI_PASSWORD;
    //static IPAddress MQTT_HOST ;
    static uint16_t MQTT_PORT ;
    static AsyncMqttClient mqttClient;
    rtos::Mutex _mtx;
    rtos::Thread _threadWiFiEvent;
    rtos::Thread _threadOnMessage;
    rtos::Thread _threadSubscribe;
    bool _autoConnect;
    //TimerHandle_t _mqttReconnectTimer;
   // TimerHandle_t _wifiReconnectTimer;
    rtos::Mail<mail_wifi_event_t, 8> _mailBoxWiFiEvent;
    rtos::Mail<mqtt::mail_t, 16> _mail_box;
    rtos::Mail<mqtt::mail_on_connect_t, 2> _mail_box_subscribe;
    std::vector<Callback<void(const String&,const String&)>>  _onMessageCallbacks;
    std::vector<Callback<void(bool)>>  _onMqttConnectCallbacks;
    std::vector<Callback<void(AsyncMqttClientDisconnectReason)>>  _onMqttDisconnectCallbacks;
    std::set<String>  _topics;
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