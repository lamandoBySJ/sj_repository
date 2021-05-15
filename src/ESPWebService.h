#ifndef __ESP_WEB_SERVICE_H
#define __ESP_WEB_SERVICE_H

#include "Arduino.h"
#include "WiFi.h"
#include <ESPAsyncWebServer.h>
#include <AsyncEventSource.h>
#include <AsyncWebSocket.h>
#include <SPIFFSEditor.h>
#include <AsyncJson.h>
#include <HTTPClient.h>
#include "ESP32httpUpdate.h"
#include <ESPmDNS.h>
#include <functional>
#include <FFat.h>
#include "platform_debug.h"
#include "FFatHelper.h"
#include <thread>
#include <mutex>
#include <app/ColorSensor/ColorSensor.h>
#include <platform/mbed.h>
#include <platform/Callback.h>
#include <RGBCollector.h>
#include <ColorConverter.h>
#include <rtos/Mutex.h>
#include "rtos/Thread.h"
#include "rtos/Mail.h"
#include "rtos/cmsis_os.h"

using namespace platform_debug;

namespace web_server
{    
    struct mail_t{
       // uint32_t counter=0;   
         uint32_t message;
    };
}
using namespace std;
class ESPWebService //: private mbed::NonCopyable<ESPWebServer>
{
public:
    ESPWebService():_thread(osPriorityNormal,1024*8),
        _server(nullptr),_events("/events"),_wss("/ws"),_handler(nullptr),
        running(false),
        _mail_box(),
        _callback(nullptr)
    {

    }
    ~ESPWebService(){
        Serial.println("~~~~~~~~~~~~~~XXX~~~~~~~~~~~~~~~~~~~~~~~~~");
        if(_handler){
            delete _handler;
        }
    }

    void startup();
    void shutdown();
    void init();
    void run_web_service();
    void notFound(AsyncWebServerRequest *request);
    void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) ;
    void post_mail(int message);

   // void delegateMethodWebSocketClientPostEvent(const String& message, const String& event, uint32_t id=0, uint32_t reconnect=0);
    bool isRunning(){
       // std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
       // lck.lock();
       // std::lock_guard<std::mutex> lck(_mtxCallback);
        return running;
    }
    //using CallbackFun = std::function<void()>;
    void runCallbackPostMailToCollector(MeasEventType measEventType,uint32_t id){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        if(this->_callback!=nullptr){
            this->_callback(measEventType,id);
        }
    }
   
    void setCallbackPostMailToCollector(mbed::Callback<void(MeasEventType,uint32_t)> callback){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_callback=callback;
    }

    void delegateMethodWebSocketClientText(uint32_t id,const String& text){
        _wss.client(id)->text(text);
    }
    void delegateMethodWebSocketClientEvent(const String& message, const String& event, uint32_t id, uint32_t reconnect){
        _events.send(message.c_str(),event.c_str(),id, reconnect);
    }

private:
    rtos::Thread _thread;
    rtos::Mutex _mtx;
    std::mutex mtx;
    AsyncWebServer* _server;
    AsyncEventSource _events;
    AsyncWebSocket _wss;
    AsyncCallbackJsonWebHandler* _handler;
    const char* PARAM_MESSAGE = "message";
    bool running;
    rtos::Mail<web_server::mail_t, 16> _mail_box;
    mbed::Callback<void(MeasEventType,uint32_t)> _callback;
    
    
    
};

#endif
