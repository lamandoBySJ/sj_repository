#ifndef LORA_GATEWAY_MASTER_H
#define LORA_GATEWAY_MASTER_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include "app/NetworkEngine/NetworkEngine.h"
#include <ArduinoJson.h>
#include <LoRaNetwork.h>
class LoRaGatewayMaster
{
public:
    LoRaGatewayMaster():_threadMqttService("mqttService",1024*6,1),
                        _threadLoraService("loraService",1024*4,1)
    {
        
    }
    void startup();
    void run_mqtt_service();
    void run_lora_service();

    void onMessageMqttCallback(const String& topic,const String& payload);
    void onMessageLoRaCallback(const String& sender,const int& rssi,const String& packet);
    void onMqttConnectCallback(bool sessionPresent);
    void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);
private:
    Thread _threadMqttService;
    Thread _threadLoraService;
    String _topicSubServerRequest;
    String _topicPubgatewayResponse;
    Mail<mqtt::mail_t,16> _mail_box_mqtt;
    Mail<lora::mail_t,16> _mail_box_lora;
    Mail<bool,16> _mail_box_connect;
};


#endif