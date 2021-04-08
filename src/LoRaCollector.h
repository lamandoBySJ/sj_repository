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
        unsigned long currentMillis;
        String TAG_ID;
} mail_t;
}
class LoRaCollector
{
public:
    LoRaCollector()=delete;
    LoRaCollector(MQTTNetwork& mqttNetwork):
        _mqttNetwork(mqttNetwork),
        _threadMqttService("mqttService",1024*4,1),
        _threadLoraService("loraService",1024*4,1),
        _threadBackgroundService("backgroundService",1024*2,1)
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
    Thread _threadMqttService;
    Thread _threadLoraService;
    Thread _threadBackgroundService;
    IPSProtocol _IPSProtocol;
    Mail<mqtt::mail_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    Mail<background::mail_t,32> _mail_box_background;
    std::map<String,std::map<String,int>> _mapTrackedDevices;
    std::set<String> _setBeaconCollector;
    std::map<String,String> _mapSetupBeacons;
    std::vector<String> _topics;
    std::map<String,std::set<String>> _mapDataCollector;
    String _topicFP;
    String _topicSendRssi;
    String _topicLT;
    String _payload;
    String  _topicCommand;
    String  _topicCommandResponse;
    rtos::Mutex _mutex;
    std::map<String,String> _mapTagLocation;
};

#endif