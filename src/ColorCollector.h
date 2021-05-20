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
namespace collector
{
    struct RGBProperties
    {   
        RGBProperties(){
            path ="/als_constant";
            r_offset = 0;
            g_offset = 0;
            b_offset = 0;
        }
        String path;
        uint16_t r_offset;
        uint16_t g_offset;
        uint16_t b_offset;

        RGBProperties& operator=(const RGBProperties& properties){
           this->path =  properties.path;
           this->r_offset = properties.r_offset;
           this->g_offset = properties.g_offset;
           this->b_offset = properties.b_offset;
           return *this;
        }
    };
    struct mail_t{
        AsyncWebSocketClient *client;
        MeasEventType eventType;
    };
    
}
using namespace collector;
using namespace platform_debug;
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
        osStatus  status = _thread.start(callback(this,&ColorCollector::run_task_collection));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_thread.get_name()):NULL);
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
         //std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
	    // lck.lock();
         std::lock_guard<rtos::Mutex> lck(_mtx);
         if(this->_cbWebSocketClientEvent!=nullptr){
            this->_cbWebSocketClientEvent("{\"status\":\""+status+"\",\"progress\":"+String(progress,DEC)+"}",event,0,0);
         }
    }
    void setCallbackWebSocketClientEvent(mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> callback){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_cbWebSocketClientEvent=callback;
    }

    void runCallbackWebSocketClientText(AsyncWebSocketClient *client,const String& text){
         std::lock_guard<rtos::Mutex> lck(_mtx);
         if(this->_cbWebSocketClientText!=nullptr){
            this->_cbWebSocketClientText(client,text);
         }
    }
    void setCallbackWebSocketClientText(mbed::Callback<void(AsyncWebSocketClient*,const String&)> callback){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_cbWebSocketClientText=callback;
    }
    
    static RGBProperties rgb_properties;
private:
    rtos::Thread _thread;
    rtos::Mutex _mtx;
    RGB _rgb;
    ColorConverter _colorConverter;
    rtos::Mail<collector::mail_t, 16> _mail_box_post;
    
   
    mbed::Callback<void(const String&, const String&,uint32_t,uint32_t)> _cbWebSocketClientEvent;
    mbed::Callback<void(AsyncWebSocketClient*,const String&)> _cbWebSocketClientText;
    
    
};

#endif