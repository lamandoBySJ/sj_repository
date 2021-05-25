#ifndef COLOR_COLLECTOR_H
#define COLOR_COLLECTOR_H
#include "Arduino.h"
#include "ArduinoJson.h"
#include "ESPWebService.h"
#include "platform_debug.h"
#include "ColorConverter.h"
#include "MQTTNetwork.h"
#include "AsyncWebSocket.h"
#include <app/ColorSensor/ColorSensor.h>
#include "rtos/Thread.h"
#include "rtos/ThisThread.h"
#include "rtos/Mail.h"
#include "platform/Callback.h"

namespace collector
{

    struct mail_ws_t{
        uint32_t id;
        AsyncWebSocketClient *client;
        MeasEventType eventType;
    };
}
class  ColorCollector
{
public:
    ColorCollector():_thread(osPriorityNormal,1024*10),doc(1024)
    {
  
    }
    ~ColorCollector()
    {
       
    }
    void invokeCallbackWebSocketClientPostEvent(int progress,const String& status , const String& event);
    void invokeCallbackWebSocketClientText(AsyncWebSocketClient *client,const String& text);
    template<typename T, typename U, typename R, typename... ArgTs>
    void setCallbackWebSocketClientEvent(U *obj,R (T::*method)(ArgTs...)){
        std::lock_guard<rtos::Mutex> lck(_mtx);
       this->_cbWebSocketClientEvent=callback(obj,method);
    }
    template<typename T, typename U, typename R, typename... ArgTs>
    void setCallbackWebSocketClientText( U *obj,R (T::*method)(ArgTs...)){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_cbWebSocketClientText=callback(obj,method);
    }

    void setCallbackWebSocketClientEvent(mbed::Callback<void(const String&, const String&, uint32_t, uint32_t)> callback){
        std::lock_guard<rtos::Mutex> lck(_mtx);
       this->_cbWebSocketClientEvent=callback;
    }
  
    void setCallbackWebSocketClientText(mbed::Callback<void(AsyncWebSocketClient*,const String&)> callback){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_cbWebSocketClientText=callback;
    }
    void setCallbackMqttPublish(mbed::Callback<bool(const String&,const String&)> callback){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_cbMqttPublish=callback;
    }
    void invokeCallbackMqttPublish(const String &topic,const String &payload)
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        if(_cbMqttPublish!=nullptr){
            _cbMqttPublish.call(topic,payload);
        }
    }
    void startup();
    void run_task_test();
    void run_task_collection();

    void delegateMethodPostMail(MeasEventType measEventType,AsyncWebSocketClient *client);
private:
    rtos::Thread _thread;
    rtos::Mutex _mtx,mtx;
    rtos::Mail<collector::mail_ws_t, 8> _mail_box_collection;
    mbed::Callback<void(AsyncWebSocketClient*,const String&)>_cbWebSocketClientText;
    mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> _cbWebSocketClientEvent;
    mbed::Callback<bool(const String&,const String&)> _cbMqttPublish;

    String text;
    DynamicJsonDocument  doc;
    RGB _rgb;
    std::array<RGB,5> arrRGB;
};

#endif