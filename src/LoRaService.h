#ifndef LORA_NETWORK_H
#define LORA_NETWORK_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include <ArduinoJson.h>
#include <heltec.h>
#include <set>
#include <iostream>
#include <sstream>
#include <atomic>
namespace lora
{
    typedef struct {
        int32_t rssi=0;   
        String sender;
        String receiver;
        String packet;
    } mail_t;
}
//using ReceiveFunctionPtr = bool(*)();
class LoRaService
{
public:
    LoRaService():_filter_broadcast(false){}
    //explicit LoRaService(bool filter_broadcast):_filter_broadcast(filter_broadcast){}
    void startup();
    void run();
    void addOnMessageCallback(mbed::Callback<void(const lora::mail_t&)> func);
    void sendMessage(const String& receiver,const String& sender,const String& packet);
    static LoRaService** getLoRaService(){
        static LoRaService* _loRaService=nullptr;
        return &_loRaService;
    }
    void set_filter_broadcast(){
        _filter_broadcast=true;
    }
    void sleep(){
        LoRa.sleep();
    }
private:
    bool _filter_broadcast;
    static void _thunkOnReceice(int packetSize);
    Thread _thread;
    rtos::Mutex _mtx;
    Mail<lora::mail_t,16> _mail_box;
    std::set<String> _beaconSet;
    std::vector<mbed::Callback<void(const lora::mail_t&)>>  _onMessageCallbacks;
    static std::atomic<long> _lastSendTime;
};

#endif