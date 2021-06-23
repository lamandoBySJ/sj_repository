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
#include "AsyncMqttClientService.h"
#include "StringHelper.h"
#include <mutex>
#include <thread>
#include "FFatHelper.h"
#include "LoRaGateway.h"
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
        String TAG_ID;
} mail_t;
}
class LoRaCollector
{
public:
    LoRaCollector()=delete;
    LoRaCollector(AsyncMqttClientService& asyncMqttClientService):
        _asyncMqttClientService(asyncMqttClientService),
        _threadMqttService(osPriorityNormal,1024*8),
        _threadLoraService(osPriorityNormal,1024*4),
        _threadBackgroundService(osPriorityNormal,1024*4),
        _mapDataCollector(),_mapOnlineDevices(),_mapSetupBeacons(),_mapTagLocation(),
        _mapTagTimeout(),_topicSplit()
    {
        

    }
    void startup(WirelessTechnologyType type=WirelessTechnologyType::WiFi);
    void run_mqtt_service();
    void run_lora_service();
    void run_background_service();

    void onMqttMessageCallback(const String& topic,const String& payload);
    void onLoRaMessageCallback(const lora::mail_t& lora_mail);
    void onMqttConnectCallback(bool sessionPresent);
    void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);

 
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
                        TracePrinter::printTrace("[DC]Setup:beaconID:"+String(p.key().c_str()));
                    }
                }
            }
        }
    }
private:
    Mail<mqtt::mail_on_message_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    Mail<background::mail_t,32> _mail_box_background_signal;
    rtos::Mutex _mtx;
    AsyncMqttClientService& _asyncMqttClientService;
    std::thread _thread;
    Thread _threadMqttService;
    Thread _threadLoraService;
    Thread _threadBackgroundService;
    
    
    std::map<String,std::set<String>> _mapDataCollector;
    std::map<String,std::map<String,int>> _mapOnlineDevices;
    std::map<String,String> _mapSetupBeacons;
    std::map<String,String> _mapTagLocation;
    std::map<String,unsigned long> _mapTagTimeout;

    std::vector<String> _topicSplit;
   // std::map<String,int> _mapTimeoutExpired;
   // String _topicTimeoutExpired;
    String _topicTimeout;
    String _topicLT;
    String _payload;
    String  _topicCommandResponse;
};

#endif