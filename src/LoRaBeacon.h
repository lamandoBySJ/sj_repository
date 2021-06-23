#ifndef LORA_BEACON_H
#define LORA_BEACON_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include <ArduinoJson.h>
#include "platform_debug.h"
#include "AsyncMqttClientService.h"
#include <LoRaService.h>
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
    BeaconProperties& operator=(BeaconProperties& other)=delete;
    BeaconProperties& operator=(BeaconProperties&& other)=delete;
    String path;
};
BeaconProperties& get_beacon_properties();
}
class LoRaBeacon
{
public:
    LoRaBeacon()=delete;
    LoRaBeacon(AsyncMqttClientService&  asyncMqttClientService):
        _asyncMqttClientService(asyncMqttClientService),
        _threadMqttService(osPriorityNormal,1024*4),
        _threadLoraService(osPriorityNormal,1024*6)
    {
       
    }
    void startup();
    void run_mqtt_service();
    void run_lora_service();
    void onLoRaMessageCallback(const lora::mail_t& lora_mail);
    void onMqttMessageCallback(const String& topic,const String& payload)
    {
         mqtt::mail_on_message_t *mail =  _mail_box_mqtt.alloc();
            if(mail!=NULL){
                mail->topic = topic;
                mail->payload = payload;
                _mail_box_mqtt.put(mail) ;
            }
    }
    /* void onMqttConnectCallback(bool sessionPresent);
    void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);
    */
  
private:
    //LoRaNetwork& _loRaNetwork;
    AsyncMqttClientService&  _asyncMqttClientService;
    Thread _threadMqttService;
    Thread _threadLoraService;
    
    String _topicSubServerRequest;
    String _topicPubgatewayResponse;
    rtos::Mail<mqtt::mail_on_message_t,16> _mail_box_mqtt;
    rtos::Mail<lora::mail_t,16> _mail_box_lora;

    
};


#endif