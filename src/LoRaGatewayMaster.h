#ifndef LORA_GATEWAY_MASTER_H
#define LORA_GATEWAY_MASTER_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include "app/NetworkEngine/NetworkEngine.h"
#include <ArduinoJson.h>

class LoRaGatewayMaster
{
public:
    LoRaGatewayMaster():_thread("gateway",1024*6,1)
    {
        
    }
    void startup();
    void run();

    void onMessageCallback(const String& topic,const String& payload);
    void attach(Callback<void(const String&,const String&)> func);
private:
    String _topicSubServerRequest;
    String _topicPubgatewayResponse;
    Thread _thread;
    Mail<network::mail_t,16> _mail_box;
    std::vector<mbed::Callback<void(const String&,const String&)>>  _debugTraceCallbacks;
};


#endif