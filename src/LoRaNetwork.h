#ifndef LORA_NETWORK_H
#define LORA_NETWORK_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include "app/NetworkEngine/NetworkEngine.h"
#include <ArduinoJson.h>
#include <heltec.h>
#include <DelegateClass.h>
#include <set>
namespace lora
{
    typedef struct {
        int32_t rssi=0;   
        String sender;
        String packet;
    } mail_t;
}
class LoRaNetwork
{
public:
    LoRaNetwork()=default;
    void startup();
    void run();
    void addOnMessageCallback(Callback<void(const String&,const int&,const String&)> func);
private:
    static LoRaNetwork* _loraNetwork;
    static void _thunkOnReceice(int packetSize);
    Thread _thread;
    Mail<lora::mail_t,16> _mail_box;
    std::set<String> _beaconSet;
    std::vector<Callback<void(const String&,const int&,const String&)>>  _onMessageCallbacks;
};

#endif