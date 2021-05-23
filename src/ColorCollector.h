#ifndef __COLOR_COLLECTOR_H
#define __COLOR_COLLECTOR_H
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
    
    struct mail_t{
        AsyncWebSocketClient *client;
        MeasEventType eventType;
    };
    struct mail_ws_t{
        uint32_t id;
        AsyncWebSocketClient *client;
        MeasEventType eventType;
    };
}
using namespace collector;
using namespace platform_debug;


class alignas(4) [[gnu::may_alias]]  ColorCollector
{
public:
    ColorCollector()
    {
  
    }
    ~ColorCollector()
    {
       
    }
    void runCallbackWebSocketClientPostEvent(int progress,const String& status , const String& event);
    void runCallbackWebSocketClientText(AsyncWebSocketClient *client,const String& text);
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

    void startup();
    void run_task_test();
    void run_task_collection();

    void delegateMethodPostMail(MeasEventType measEventType,AsyncWebSocketClient *client);
private:
    rtos::Thread _thread;
    rtos::Thread _thread2;
    rtos::Mutex _mtx,mtx;
    rtos::Mail<mail_ws_t, 8> _mail_box_collection;
    alignas(128)  mbed::Callback<void(AsyncWebSocketClient*,const String&)>_cbWebSocketClientText;
    alignas(128)  mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> _cbWebSocketClientEvent;
};

#endif