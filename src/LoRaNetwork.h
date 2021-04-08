#ifndef LORA_NETWORK_H
#define LORA_NETWORK_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include "MQTTNetwork.h"
#include <ArduinoJson.h>
#include <heltec.h>
#include <DelegateClass.h>
#include <set>
#include <iostream>
#include <sstream>
namespace lora
{
    typedef struct {
        int32_t rssi=0;   
        String sender;
        String receiver;
        String packet;
    } mail_t;
}
class LoRaNetwork
{
public:
    LoRaNetwork()=default;
    void startup();
    void run();
    void addOnMessageCallback(Callback<void(const lora::mail_t&)> func);
    void sendMessage(const String& receiver,const String& sender,const String& packet);
private:
    static LoRaNetwork* _loraNetwork;
    static void _thunkOnReceice(int packetSize);
    Thread _thread;
    Mail<lora::mail_t,16> _mail_box;
    std::set<String> _beaconSet;
    std::vector<Callback<void(const lora::mail_t&)>>  _onMessageCallbacks;
    static rtos::Mutex _mutex;
    static long _lastSendTime;
};

#endif