#ifndef DATA_COLLECTOT_H
#define DATA_COLLECTOT_H

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

namespace ml
{
    typedef struct {
        uint32_t counter=0;   
        String TAG_ID;
} mail_t;
}
class DataCollector
{
public:
    DataCollector()=delete;
    DataCollector(MQTTNetwork& mqttNetwork):
        _mqttNetwork(mqttNetwork),
        _threadMqttService("mqttService",1024*4,1),
        _threadLoraService("loraService",1024*4,1)
    {
        _topics.push_back(String("Command/Request/DataCollector"));
    }
    void startup();
    void run_mqtt_service();
    void run_lora_service();

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

    Mail<mqtt::mail_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    Mail<ml::mail_t,32> _mail_box_ml;
    std::map<String,std::map<String,int>> _mapBeacons;
    std::map<String,int> _mapRssiCounter;
    std::vector<String> _topics;
    String _fingerprints;
};

#endif