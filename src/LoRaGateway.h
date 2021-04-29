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
#include "FFatHelper.h"

using namespace platform_debug;
class LoRaGateway
{
public:
    LoRaGateway()=delete;
    LoRaGateway(MQTTNetwork& mqttNetwork,LoRaNetwork& loRaNetwork):
        _mqttNetwork(mqttNetwork),
        _loRaNetwork(loRaNetwork),
        _threadMqttService("mqttService",1024*4,1),
        _threadLoraService("loraService",1024*4,1),
        _IPSProtocol(),
        _mail_box_mqtt(),_mail_box_lora(),
        _mapSetupBeacons(),_mapRetry(),_mapTagLocation(),
        _topicSplit(),_topics()

    {
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

    void setupBeacons(const String& text){
        DynamicJsonDocument  doc(text.length()+1024);
        DeserializationError error = deserializeJson(doc,text);
        if(!error){
            if(doc.containsKey("beacons")){
                for(auto v :doc["beacons"].as<JsonArray>()){
                    for(JsonPair p : v.as<JsonObject>()){
                        _mapSetupBeacons[p.key().c_str()]=p.value().as<String>();
                        platform_debug::TracePrinter::printTrace("[GW]MQTT:beaconID:"+String(p.key().c_str()));
                    }
                }
            }
        }
    }
private:
    MQTTNetwork& _mqttNetwork;
    LoRaNetwork& _loRaNetwork;
    Thread _threadMqttService;
    Thread _threadLoraService;
    IPSProtocol _IPSProtocol;
   
    Mail<mqtt::mail_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    std::map<String,String> _mapSetupBeacons;
    std::map<String,int> _mapRetry;
    std::map<String,String> _mapTagLocation;
    std::vector<String> _topicSplit;
    std::vector<String> _topics;

    String _topicSubServerRequest;
    String _topicPubgatewayResponse;
    String _topicLearn;
    String _topicTrack;
    String _topicCommandRequest;
    String _topicTimeout;
    String _topicCommandResponse;
    String _mode;
    String  _topicLT;
    String _payload;
    
    
};


#endif