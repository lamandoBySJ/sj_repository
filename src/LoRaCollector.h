#ifndef LORA_COLLECTOT_H
#define LORA_COLLECTOT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include "MQTTNetwork.h"
#include <LoRaNetwork.h>
#include "StringHelper.h"
#include <thread>
using namespace platform_debug;
enum class WirelessTechnologyType
{
    WiFi = 0,
    LoRa,
    Ble
};
namespace background
{
    typedef struct {
        uint32_t counter=0;  
        //unsigned long currentMillis;
        String TAG_ID;
} mail_t;
}
class LoRaCollector
{
public:
    LoRaCollector()=delete;
    LoRaCollector(MQTTNetwork& mqttNetwork): _mqttNetwork(mqttNetwork)
    {
        _mapSetupBeacons[String("9F8C")] = String("A001");
       // _mapSetupBeacons[String("A18C")] = String("A002");
    }
    void startup(WirelessTechnologyType type=WirelessTechnologyType::WiFi);
    void run_mqtt_service();
    void run_lora_service();
    void run_background_service();

    void onMessageMqttCallback(const String& topic,const String& payload);
    void onMessageLoRaCallback(const lora::mail_t& lora_mail);
    void onMqttConnectCallback(bool sessionPresent);
    void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);

    std::vector<String>& getTopics(){
        return _topics;
    }
private:
    MQTTNetwork& _mqttNetwork;
    std::thread _threadMqttService;
    std::thread _threadLoraService;
    std::thread _threadBackgroundService;
    IPSProtocol _IPSProtocol;
    Mail<mqtt::mail_message_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    Mail<background::mail_t,32> _mail_box_background;
    Mail<int,16> _mail_box_signal;
    std::map<String,std::map<String,int>> _mapOnlineDevices;
    std::map<String,String> _mapSetupBeacons;
    std::vector<String> _topics;
    std::map<String,std::set<String>> _mapDataCollector;
    std::map<String,int> _mapTimeoutExpired;
    String _topicTimeout;
    String _topicTimeoutExpired;
    String _topicSendRssi;
    String _topicLT;
    String _payload;
    String  _topicCommand;
    String  _topicCommandResponse;
    rtos::Mutex _mutex;
    std::map<String,String> _mapTagLocation;
    std::vector<String> _topicSplit;
    std::vector<String> _topicMatch;
    long _millis;
};

#endif