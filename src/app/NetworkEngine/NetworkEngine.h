#pragma once

#include <arduino.h>
#include <app/AsyncMqttClient/AsyncMqttClient.h>
#include <WiFiType.h>
#include <WiFi.h>
#include <rtos/rtos.h>
#include "DelegateClass.h"
#include "platform_debug.h"
#include <map>
#include <new>
namespace mail
{
enum class MqttEvent : char 
{
  onMqttConnect=1,
  onMqttDisconnect,
  onMqttSubscribe,
  onMqttUnsubscribe,
  onMqttMessage,
  onMqttPublish
};
enum class EventType : char 
{
  WiFi=0,
  MQTT
};
typedef union {
    system_event_id_t wifi_event;
    MqttEvent mqtt_event; 
} event_t;

typedef struct {
    uint32_t counter=0;   
    String message;
} mail_trace_t;
typedef struct {
    uint32_t counter=0;   
    AsyncMqttClientMessageProperties properties;
    size_t index;
    size_t total;
    String topic;
    String payload;
} mail_box_t;
}

using namespace mail;
class NetworkEngine
{
public:
    NetworkEngine()
    {

    }
    NetworkEngine(const NetworkEngine& other)=default;
    NetworkEngine(NetworkEngine&& other)=default;
    ~NetworkEngine()=default;

    NetworkEngine& operator = (const NetworkEngine& that)=default;
    NetworkEngine& operator = (NetworkEngine&& that)=default;

    void onMqttConnect(bool sessionPresent);

    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

    void onMqttSubscribe(uint16_t packetId, uint8_t qos);

    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(char* topic,char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) ;
    //void onMqttMessage(const String& topic,const String& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);

    void setMqttReconnectTimer(bool start);
    void setWifiReconnectTimer(bool start);

    void startup();
    void run_mail_box();
    void run_debug_trace();

    void _connectToMqtt();
    void _connectToWifi();
    //bool publish(const char* topic,String& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool publish(String& topic,String& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool publish(String& topic,const String&& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool publish(const String&& topic,String& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool publish(const String&& topic,const String&& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);

    bool subscribe(const char* topic, uint8_t qos=0);
    bool subscribe(String& topic, uint8_t qos=0);
    bool subscribe(const String&& topic, uint8_t qos=0);

    void WiFiEvent(system_event_id_t event, system_event_info_t info);
    void attach(Callback<void(const String&,const String&)> func);
    void addOnMessageCallback(Callback<void(const String&,const String&)> func);
    
    
private:
   // void onMessageCallback(const String& topic,const String& payload);
    static void _thunkConnectToWifi(void* pvTimerID);
    static void _thunkConnectToMqtt(void* pvTimerID);
    inline void printTrace(const String& e);
    inline void printTrace(const char* e);
    
    TimerHandle_t _mqttReconnectTimer;
    TimerHandle_t _wifiReconnectTimer;
    static const char* WIFI_SSID;
    static const char* WIFI_PASSWORD;
    static IPAddress MQTT_HOST ;
    static uint16_t MQTT_PORT ;
    static AsyncMqttClient mqttClient;
    static rtos::Mutex _mutex,std_mutex;
    bool _connected=false;
    system_event_id_t _event=SYSTEM_EVENT_WIFI_READY;
    //system_event_info_t _info;
    #if !defined(NDEBUG)
    rtos::Mail<mail_trace_t, 16> _mail_box_debug_trace;
    std::vector<Callback<void(const String&,const String&)>>  _debugTraceCallbacks;
    Thread _threadDebug;
    #endif
    rtos::Mail<mail_box_t, 16> _mail_box;
    std::vector<Callback<void(const String&,const String&)>>  _onMessageCallbacks;
    
    Thread _threadMail;
    
};

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