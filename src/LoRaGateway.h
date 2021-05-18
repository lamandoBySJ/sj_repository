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
#include "StringHelper.h"
#include <thread>
using namespace platform_debug;
class LoRaGateway
{
public:
    LoRaGateway()=delete;
    LoRaGateway(MQTTNetwork& mqttNetwork,LoRaNetwork& loRaNetwork):
        _mqttNetwork(mqttNetwork),
        _loRaNetwork(loRaNetwork)
    {
         _mapSetupBeacons[String("9F8C")] = String("A001");
         _mode="learn";
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
    std::thread _threadMqttService;
    std::thread _threadLoraService;
    IPSProtocol _IPSProtocol;
    String _topicSubServerRequest;
    String _topicPubgatewayResponse;
    Mail<mqtt::mail_message_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    std::vector<String> _topics;
    String _topicLearn;
    String _topicTrack;
    String _topicCommandRequest;
    String _topicTimeout;
    String _topicCommandResponse;
    std::map<String,String> _mapSetupBeacons;
    std::map<String,int> _mapRetry;
    String _mode;
    String  _topicLT;
    String _payload;
    std::map<String,String> _mapTagLocation;
    std::vector<String> _topicMatch;
    std::vector<String> _topicSplit;
    
};


#endif