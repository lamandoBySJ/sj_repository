#pragma once
#include "AsyncMqttClientService.h"

#include "ESPWebService.h"
#include "drivers/Timeout.h"
#include "WiFiService.h"
#include "LoRaTag.h"
#include "LoRaService.h"
#include "LoRaCollector.h"
#include "LoRaBeacon.h"
#include "LoRaGateway.h"

#include "LoRaService.h"
class IPSBox
{
public:
    IPSBox():_wifiService()
    ,_asyncMqttClientService()
    ,_loRaService()
    ,_loRaCollector(_asyncMqttClientService)
    ,_loRaBeacon(_asyncMqttClientService)
    ,_loRaGateway(_asyncMqttClientService,_loRaService)
    ,_loraTag(_loRaService)
    ,_mtx_i2c_bus()
   // ,_rtc(_mtx_i2c_bus,Wire1)
    //,_mcp23017(_mtx_i2c_bus,Wire1)
    ,_topics()
    {

    }
    ~IPSBox()=default;

     //_asyncMqttClientService.addOnMqttMessageCallback(mbed::callback(this,&IPSBox::onMqttMessageCallback));
     //_asyncMqttClientService.addOnMqttSubscribeCallback(mbed::callback(this,&IPSBox::onMqttSubscribeCallback));   

    
    void platformio_init();

    void lora_collector_init();
    void lora_gateway_init();
    void lora_beacon_init();
    void lora_tag_init();
    void onMqttConnectCallback(bool sessionPresent);
    void onMqttMessageCallback(const String& topic,const String& payload);
    void onMqttSubscribeCallback(uint16_t packetId, uint8_t qos);
    void onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason);
    
    void start_web_service();


    void i2c_devices_init();
    void restart(){
        ESP.restart();
    }
    void onWiFiServiceCallback(const mail_wifi_event_t& wifi_event){
        switch(wifi_event.event_id){
        case SYSTEM_EVENT_STA_STOP:
            break;
        case SYSTEM_EVENT_STA_START:
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            _asyncMqttClientService.connect();
            TracePrinter::printTrace("MQTT Connecting...");
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            break;
        default:
            break;
        } 
    }
   void stop_lora_tag(){
       _loraTag.shutdown();
   }
   void start(){
       _asyncMqttClientService.startup();
        _wifiService.addOnWiFiServiceCallback(mbed::callback(this,&IPSBox::onWiFiServiceCallback));
        _wifiService.startup();
   }
   void publish(const String& text ){
        _asyncMqttClientService.publish("time",text);
   }
private:
    WiFiService _wifiService;
    ESPWebService _espWebService;
    AsyncMqttClientService _asyncMqttClientService;
    LoRaService _loRaService;

    LoRaCollector _loRaCollector;
    LoRaBeacon _loRaBeacon;
    LoRaGateway _loRaGateway;
    LoRaTag _loraTag;

    rtos::Mutex _mtx_i2c_bus;
    std::vector<String>  _topics;
    mbed::Timeout _boot_flipper;
    
    
    static uint32_t _time_sync_counter;
};
