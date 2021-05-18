#ifndef __COLOR_COLLECTOR_H
#define __COLOR_COLLECTOR_H
#include "Arduino.h"
#include "ArduinoJson.h"
#include "ESPWebService.h"
#include <app/ColorSensor/ColorSensor.h>
#include "platform_debug.h"
#include "ColorConverter.h"
#include "MQTTNetwork.h"
#include "AsyncWebSocket.h"
 //using WebServerEventSourceCallback = void(ESPWebServer::*)(const String& message, const String& event, uint32_t id, uint32_t reconnect);
namespace collector
{
    struct mail_t{
        AsyncWebSocketClient *client;
        MeasEventType eventType;
        //String message;
    };
    
}
using namespace collector;



class ColorCollector
{
public:

    ColorCollector():
        _thread(osPriorityNormal,1024*6),
        _rgb(),_colorConverter(),
        _cbWebSocketClientEvent(nullptr),
        _cbWebSocketClientText(nullptr)
    {

    }

    ~ColorCollector()
    {
       
    }
   
    void startup()
    {  
        _thread.start(callback(this,&ColorCollector::run_task_collection));
    }
    
    void run_task_collection();

    void delegateMethodPostMail(MeasEventType measEventType,AsyncWebSocketClient* client)
    {   
        collector::mail_t *mail = _mail_box_post.alloc();
        if(mail!=NULL){
            mail->client = client;
            mail->eventType = measEventType;
            _mail_box_post.put(mail) ;
        }
    }
    void runCallbackWebSocketClientPostEvent(int progress,const String& status , const String& event){
         //std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
	    // lck.lock();
         std::lock_guard<std::mutex> lck(_mtx);
         if(this->_cbWebSocketClientEvent!=nullptr){
            this->_cbWebSocketClientEvent("{\"status\":\""+status+"\",\"progress\":"+String(progress,DEC)+"}",event,0,0);
         }
    }
    void setCallbackWebSocketClientEvent(mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> callback){
        std::lock_guard<std::mutex> lck(_mtx);
        this->_cbWebSocketClientEvent=callback;
    }

    void runCallbackWebSocketClientText(AsyncWebSocketClient *client,const String& text){
         std::lock_guard<std::mutex> lck(_mtx);
         if(this->_cbWebSocketClientText!=nullptr){
               platform_debug::TracePrinter::printTrace("ESP:::A:::::"+text);
            this->_cbWebSocketClientText(client,text);
         }
    }
    void setCallbackWebSocketClientText(mbed::Callback<void(AsyncWebSocketClient*,const String&)> callback){
        std::lock_guard<std::mutex> lck(_mtx);
        this->_cbWebSocketClientText=callback;
    }
private:
    rtos::Thread _thread;
    RGB _rgb;
    ColorConverter _colorConverter;
    mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> _cbWebSocketClientEvent;
    mbed::Callback<void(AsyncWebSocketClient*,const String&)> _cbWebSocketClientText;
    std::mutex _mtx;
    rtos::Mail<collector::mail_t, 16> _mail_box_post;
    
};

#endif