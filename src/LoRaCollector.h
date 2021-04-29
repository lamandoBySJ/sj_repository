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
#include <mutex>
#include <thread>
#include "FFatHelper.h"
using namespace platform_debug;
enum class WirelessTechnologyType
{
    WiFi = 0,
    LoRa,
    Ble
};
enum class TaskState {
        Ready,              /**< Ready to run */
        Running,            /**< Running */
};
namespace background
{
    typedef struct {
        uint32_t counter=0;  
        //unsigned long currentMillis;
        String TAG_ID;
        // char TAG_ID[13];
} mail_t;
}
class LoRaCollector
{
public:
    LoRaCollector()=delete;
    LoRaCollector(MQTTNetwork& mqttNetwork):
        _mqttNetwork(mqttNetwork),
        _threadMqttService("mqttService",1024*8,1),
        _threadLoraService("loraService",1024*4,1),
        _threadBackgroundService("backgroundService",1024*4,1),
        _mtx(),_mtxMillisec(),_IPSProtocol(),
        _mail_box_mqtt(),_mail_box_lora(),_mail_box_background_signal(),
        _mapDataCollector(),_mapOnlineDevices(),_mapSetupBeacons(),_mapTagLocation(),
        _mapTagTimeout(),_topics(),_topicSplit(),_taskState(TaskState::Ready)
    {
        

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
    void setupBeacons(const String& text){
        DynamicJsonDocument  doc(text.length()+1024);
        DeserializationError error = deserializeJson(doc,text);
        if(!error){
            if(doc.containsKey("beacons")){
                for(auto v :doc["beacons"].as<JsonArray>()){
                    for(JsonPair p : v.as<JsonObject>()){
                        _mapSetupBeacons[p.key().c_str()]=p.value().as<String>();
                        platform_debug::TracePrinter::printTrace("[DC]MQTT:beaconID:"+String(p.key().c_str()));
                    }
                }
            }
        }
    }
private:
  
    MQTTNetwork& _mqttNetwork;
    std::thread _thread;
    Thread _threadMqttService;
    Thread _threadLoraService;
    Thread _threadBackgroundService;
    std::mutex _mtx;
    std::mutex _mtxMillisec;
    IPSProtocol _IPSProtocol;
    Mail<mqtt::mail_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    Mail<background::mail_t,32> _mail_box_background_signal;
    std::map<String,std::set<String>> _mapDataCollector;
    std::map<String,std::map<String,int>> _mapOnlineDevices;
    std::map<String,String> _mapSetupBeacons;
    std::map<String,String> _mapTagLocation;
    std::map<String,unsigned long> _mapTagTimeout;
    std::vector<String> _topics;
    std::vector<String> _topicSplit;
   // std::map<String,int> _mapTimeoutExpired;
   // String _topicTimeoutExpired;
    String _topicTimeout;
   
    String _topicSendRssi;
    String _topicLT;
    String _payload;
    String  _topicCommandRequest;
    String  _topicCommandResponse;
    
    long _millis;
    TaskState _taskState;
};

#endif