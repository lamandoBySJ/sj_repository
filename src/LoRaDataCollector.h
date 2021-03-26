#ifndef LORA_DATA_COLLECTOT_H
#define LORA_DATA_COLLECTOT_H

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

namespace background
{
    typedef struct {
        uint32_t counter=0;   
        String TAG_ID;
} mail_t;
}
class LoRaDataCollector
{
public:
    LoRaDataCollector()=delete;
    LoRaDataCollector(MQTTNetwork& mqttNetwork):
        _mqttNetwork(mqttNetwork),
        _threadMqttService("mqttService",1024*4,1),
        _threadLoraService("loraService",1024*4,1),
        _threadBackgroundService("backgroundService",1024*2,1)
    {
        _topics.push_back(String("Command/Request/LoRaDataCollector"));
        _mapSetupBeacons[String("9F8C")] = String("A001");
        _mapSetupBeacons[String("A18C")] = String("A002");

    }
    void startup();
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

    Mail<mqtt::mail_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    Mail<background::mail_t,32> _mail_box_background;
    std::map<String,std::map<String,int>> _mapTrackDevices;
    std::set<String> _setBeaconCollector;
    std::map<String,String> _mapSetupBeacons;
    std::vector<String> _topics;
    std::map<String,std::set<String>> _mapTagCounter;
    String _fingerprints;
    String _topicSendRssi;
    String _topicLT;
    rtos::Mutex _mutex;
};

#endif