#ifndef __RGB_COLLECTOR_H
#define __RGB_COLLECTOR_H
#include "Arduino.h"
#include "ArduinoJson.h"
#include "ESPWebService.h"
#include <app/ColorSensor/ColorSensor.h>
#include "platform_debug.h"
#include "ColorConverter.h"
#include "MQTTNetwork.h"
namespace collector
{
    struct mail_t{
        uint32_t id=0;
        MeasEventType eventType;
        //String message;
    };
    
}
using namespace collector;

template<class T>
class RGBCollector //: private mbed::NonCopyable<RGBCollector>
{
public:

    RGBCollector(MQTTNetwork& MQTTnetwork,ColorSensor<T>& colorSensor):
        _MQTTnetwork(MQTTnetwork),_colorSensor(colorSensor),
        _rgb(),_colorConverter(),_mail_box(),
        _cbWebSocketClientEvent(nullptr),
        _cbWebSocketClientText(nullptr)
    {

    }

    ~RGBCollector()
    {

    }
    //using WebServerEventSourceCallback = void(ESPWebServer::*)(const String& message, const String& event, uint32_t id, uint32_t reconnect);
    void startup();
    void run_task_collection();
    void delegateMethodPostMail(MeasEventType measEventType,uint32_t id=0);

    void runCallbackWebSocketClientPostEvent(const String& message, const String& event){
         //std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
	    // lck.lock();
         std::lock_guard<std::mutex> lck(_mtx);
         if(this->_cbWebSocketClientEvent!=nullptr){
            this->_cbWebSocketClientEvent(message,event,0,0);
         }
    }
    void setCallbackWebSocketClientEvent(mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> callback){
        std::lock_guard<std::mutex> lck(_mtx);
        this->_cbWebSocketClientEvent=callback;
    }

    void runCallbackWebSocketClientText(uint32_t id,const String& text){
         std::lock_guard<std::mutex> lck(_mtx);
         if(this->_cbWebSocketClientText!=nullptr){
            this->_cbWebSocketClientText(id,text);
         }
    }
    void setCallbackWebSocketClientText(mbed::Callback<void(uint32_t,const String&)> callback){
        std::lock_guard<std::mutex> lck(_mtx);
        this->_cbWebSocketClientText=callback;
    }
private:
    MQTTNetwork& _MQTTnetwork;
    ColorSensor<T>& _colorSensor;
    RGB _rgb;
    ColorConverter _colorConverter;
    rtos::Mail<collector::mail_t, 16> _mail_box;
    mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> _cbWebSocketClientEvent;
    mbed::Callback<void(uint32_t id,const String& text)> _cbWebSocketClientText;
    std::mutex _mtx;
    std::thread _thread;
};

#endif