#ifndef RGB_COLLECTOR_H
#define RGB_COLLECTOR_H
#include "Arduino.h"
#include "ArduinoJson.h"
#include "ESPWebServer.h"
#include <app/ColorSensor/ColorSensor.h>
#include "platform_debug.h"
#include "ColorConverter.h"


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
    
    RGBCollector(MQTTNetwork& MQTTnetwork,ColorSensor<T>& colorSensor);
    ~RGBCollector();
    //using WebServerEventSourceCallback = void(ESPWebServer::*)(const String& message, const String& event, uint32_t id, uint32_t reconnect);
    void startup();
    void run_collector();
    void delegateMethodPostMail(MeasEventType measEventType,uint32_t id=0);

    void runCallbackWebSocketClientPostEvent(const String& message, const String& event){
         std::unique_lock<std::mutex> lck(_mtxCallback, std::defer_lock);
	     lck.lock();
         if(this->_cbWebSocketClientEvent!=nullptr){
            this->_cbWebSocketClientEvent(message,event,0,0);
         }
    }
    void setWebSocketClientEventCallback(mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> callback){
        std::unique_lock<std::mutex> lck(_mtxCallback, std::defer_lock);
	    lck.lock();
        this->_cbWebSocketClientEvent=callback;
    }

    void runCallbackWebSocketClientText(uint32_t id,const String& text){
         std::unique_lock<std::mutex> lck(_mtxCallback, std::defer_lock);
	     lck.lock();
         if(this->_cbWebSocketClient!=nullptr){
            this->_cbWebSocketClient(id,text);
         }
    }
    void setWebSocketClientTextCallback(mbed::Callback<void(uint32_t,const String&)> callback){
        std::unique_lock<std::mutex> lck(_mtxCallback, std::defer_lock);
	    lck.lock();
        this->_cbWebSocketClient=callback;
    }
private:
    MQTTNetwork& _MQTTnetwork;
    ColorSensor<T>& _colorSensor;
    RGB _rgb;
    RGB _rgbTemp;
    rtos::Mail<collector::mail_t, 16> _mail_box;
    std::mutex _mtx,_mtxCallback;
    std::thread _thread;
    mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> _cbWebSocketClientEvent;
    mbed::Callback<void(uint32_t id,const String& text)> _cbWebSocketClient;
    ColorConverter _colorConverter;
};

#endif