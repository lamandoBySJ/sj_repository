#pragma once

#include <arduino.h>
#include <app/AsyncMqttClient/AsyncMqttClient.h>
#include <WiFiType.h>
#include <WiFi.h>
#include <rtos/rtos.h>


class NetworkEngine
{
public:
    NetworkEngine()=default;
    NetworkEngine(const NetworkEngine& other)=default;
    NetworkEngine(NetworkEngine&& other)=default;
    ~NetworkEngine()=default;

    NetworkEngine& operator = (const NetworkEngine& that)=default;
    NetworkEngine& operator = (NetworkEngine&& that)=default;

    void onMqttConnect(bool sessionPresent);

    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

    void onMqttSubscribe(uint16_t packetId, uint8_t qos);

    void onMqttUnsubscribe(uint16_t packetId);
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
    void onMqttMessage(String&& topic,String&& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);

    void setMqttReconnectTimer(bool start);
    void setWifiReconnectTimer(bool start);

    void startup();
    void run();
  
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
private:
    static void _thunkConnectToWifi(void* pvTimerID);
    static void _thunkConnectToMqtt(void* pvTimerID);
    
    TimerHandle_t _mqttReconnectTimer;
    TimerHandle_t _wifiReconnectTimer;
    static const char* WIFI_SSID;
    static const char* WIFI_PASSWORD;
    static IPAddress MQTT_HOST ;
    static uint16_t MQTT_PORT ;
    static AsyncMqttClient mqttClient;
    static rtos::Mutex _mutex;
    bool _connected=false;
    system_event_id_t _event=SYSTEM_EVENT_WIFI_READY;
    //system_event_info_t _info;
};

