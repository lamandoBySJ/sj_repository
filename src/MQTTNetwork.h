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
struct user_properties{
static String path ;
static String ssid ;
static String pass ;
static String host ;
static int    port ;
};
}

using namespace mqtt;
using namespace platform_debug;
class MQTTNetwork
{
public:
    MQTTNetwork():_threadWiFiEvent(osPriorityNormal,1024*2),
    _threadSubscribe(osPriorityNormal,1024*2),
    _threadOnMessage(osPriorityNormal,1024*4),
    _mtx(),_autoConnect(true)
    {
        
    }
    MQTTNetwork(const MQTTNetwork& other)=default;
    MQTTNetwork(MQTTNetwork&& other)=default;
    ~MQTTNetwork()=default;

    MQTTNetwork& operator = (const MQTTNetwork& that)=default;
    MQTTNetwork& operator = (MQTTNetwork&& that)=default;

    void onMqttConnect(bool sessionPresent);

    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

    void onMqttSubscribe(uint16_t packetId, uint8_t qos);

    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(char* topic,char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) ;
    //void onMqttMessage(const String& topic,const String& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);

   // void setMqttReconnectTimer(bool start);
   // void setWifiReconnectTimer(bool start);

    void startup();
    void runWiFiEventService();
    void run_mail_box();
    void run_mail_box_subscribe();
    void run_debug_trace();
    bool connected();
    void _connectToMqtt();
    void _connectToWifi();
    void disconnect(bool autoConnect=false);
    //bool publish(const char* topic,String& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool publish(const String& topic,const String& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool subscribe(const String& topic, uint8_t qos=0);
    bool unsubscribe(const String& topic);
   
    void WiFiEvent(system_event_id_t event, system_event_info_t info);
    void addOnMessageCallback(Callback<void(const String&,const String&)> func);
    void addSubscribeTopic(const String& topic,int qos=0);
    void addSubscribeTopics(std::vector<String>& vec);
    void addOnMqttConnectCallback(Callback<void(bool)> func);
    void addOnMqttDisonnectCallback(Callback<void(AsyncMqttClientDisconnectReason)> func);

    static void _thunkConnectToWifi(void* pvTimerID);
    static void _thunkConnectToMqtt(void* pvTimerID);
    inline void printTrace(const String& e);
    inline void printTrace(const char* e);
private:
    static const char* MQTT_HOST;
    static const char* WIFI_SSID;
    static const char* WIFI_PASSWORD;
    //static IPAddress MQTT_HOST ;
    static uint16_t MQTT_PORT ;
    static AsyncMqttClient mqttClient;
   // void onMessageCallback(const String& topic,const String& payload);
    Thread _threadWiFiEvent;
    Thread _threadSubscribe;
    Thread _threadOnMessage;
    std::mutex _mtx;
    bool _autoConnect;
    TimerHandle_t _mqttReconnectTimer;
    TimerHandle_t _wifiReconnectTimer;
    
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