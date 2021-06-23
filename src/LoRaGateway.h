#ifndef LORA_GATEWAY_H
#define LORA_GATEWAY_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include "AsyncMqttClientService.h"
#include <ArduinoJson.h>
#include <LoRaService.h>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include "StringHelper.h"
#include "FFatHelper.h"
#include "LoRaBeacon.h"
namespace product_api
{
    struct IPSProtocol
    {
        IPSProtocol(){
            technology="LoRa";
            family="k49a";
            gateway="9F8C";
            collector="9F8C";
            mode = "learn";
        }
        String technology;
        String family;
        String gateway;
        String collector;
        String mode;
    };
    IPSProtocol& get_ips_protocol();
}

class LoRaGateway
{
public:
    LoRaGateway()=delete;
    LoRaGateway(AsyncMqttClientService& asyncMqttClientService,LoRaService& loRaService):
        _asyncMqttClientService(asyncMqttClientService),
        _loRaService(loRaService),
        _threadMqttService(osPriorityNormal,1024*4),
        _threadLoraService(osPriorityNormal,1024*4),
        _mail_box_mqtt(),_mail_box_lora(),
        _mapSetupBeacons(),_mapRetry(),_mapTagLocation(),
        _topicSplit(),_topics()

    {
         _mode="learn";
    }
    void startup();
    void run_mqtt_service();
    void run_lora_service();

    void onMqttMessageCallback(const String& topic,const String& payload);
    void onLoRaMessageCallback(const lora::mail_t& lora_mail);
    void onMqttConnectCallback(bool sessionPresent);
    void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);
    std::vector<String>& getTopics(){
        return _topics;
    }

    void setupBeacons(){
        String text ="";
        if(!FatHelper.readFile(FFat,product_api::get_beacon_properties().path,text)){
            text="{\"beacons\":[{\"15BC\":\"a\"},{\"15E8\":\"b\"},{\"1570\":\"c\"},{\"16BC\":\"d\"},{\"1594\":\"e\"}]}";
        }
        DynamicJsonDocument  doc(text.length()+1024);
        DeserializationError error = deserializeJson(doc,text);
        if(!error){
            if(doc.containsKey("beacons")){
                for(auto v :doc["beacons"].as<JsonArray>()){
                    for(JsonPair p : v.as<JsonObject>()){
                        _mapSetupBeacons[p.key().c_str()]=p.value().as<String>();
                       TracePrinter::printTrace("[GW]MQTT:beaconID:"+String(p.key().c_str()));
                    }
                }
            }
        }
    }
private:
    AsyncMqttClientService& _asyncMqttClientService;
    LoRaService& _loRaService;
    Thread _threadMqttService;
    Thread _threadLoraService;
  
    Mail<mqtt::mail_on_message_t,16> _mail_box_mqtt;
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