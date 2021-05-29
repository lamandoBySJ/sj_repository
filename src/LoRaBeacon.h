#ifndef LORA_BEACON_H
#define LORA_BEACON_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include <ArduinoJson.h>
#include "platform_debug.h"
#include "NetworkService.h"
#include <LoRaNetwork.h>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

using namespace lora;

namespace product_api
{
struct BeaconProperties{
    BeaconProperties()
    {
        path="/beacon_properties";
    }
    String path;
};
BeaconProperties& get_beacon_properties();
}
class LoRaBeacon
{
public:
    LoRaBeacon()=delete;
    LoRaBeacon(NetworkService& networkService):
        _networkService(networkService),
        _threadMqttService(osPriorityNormal,1024*4),
        _threadLoraService(osPriorityNormal,1024*6)
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
    NetworkService& _networkService;
    Thread _threadMqttService;
    Thread _threadLoraService;
    
    String _topicSubServerRequest;
    String _topicPubgatewayResponse;
    Mail<mqtt::mail_on_message_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    std::vector<String> _topics;
    String _topicCommand;
};


#endif