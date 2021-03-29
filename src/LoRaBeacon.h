#ifndef LORA_BEACON_H
#define LORA_BEACON_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include <ArduinoJson.h>
#include "platform_debug.h"
#include "MQTTnetwork.h"
#include <LoRaNetwork.h>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

using namespace lora;
using namespace platform_debug;
class LoRaBeacon
{
public:
    LoRaBeacon()=delete;
    LoRaBeacon(MQTTNetwork& mqttNetwork):
        _mqttNetwork(mqttNetwork),
        _threadMqttService("mqttService",1024*4,1),
        _threadLoraService("loraService",1024*6,1)
    {
       
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
    //LoRaNetwork& _loRaNetwork;
    MQTTNetwork& _mqttNetwork;
    Thread _threadMqttService;
    Thread _threadLoraService;
    
    String _topicSubServerRequest;
    String _topicPubgatewayResponse;
    Mail<mqtt::mail_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    std::vector<String> _topics;
    String _topicCommand;
};


#endif