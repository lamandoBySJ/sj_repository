#ifndef LORA_GATEWAY_H
#define LORA_GATEWAY_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include "MQTTNetwork.h"
#include "LoRaNetwork.h"
#include <ArduinoJson.h>
#include <LoRaNetwork.h>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
class LoRaGateway
{
public:
    LoRaGateway()=delete;
    LoRaGateway(MQTTNetwork& mqttNetwork,LoRaNetwork& loRaNetwork):
        _mqttNetwork(mqttNetwork),
        _loRaNetwork(loRaNetwork),
        _threadMqttService("mqttService",1024*6,1),
         _threadLoraService("loraService",1024*4,1)
    {
        _topics.push_back(String("Command/Request/LoRaGateway"));
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
    LoRaNetwork& _loRaNetwork;
    Thread _threadMqttService;
    Thread _threadLoraService;
    
    String _topicSubServerRequest;
    String _topicPubgatewayResponse;
    Mail<mqtt::mail_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    std::vector<String> _topics;
};


#endif