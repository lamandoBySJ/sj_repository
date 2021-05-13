#ifndef ESP_WEB_SERVER
#define ESP_WEB_SERVER
#include "Arduino.h"
#include "WiFi.h"
#include <ESPAsyncWebServer.h>
#include <AsyncEventSource.h>
#include <AsyncWebSocket.h>
#include <SPIFFSEditor.h>
#include <AsyncJson.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <functional>
#include <FFat.h>
#include "platform_debug.h"
#include "FFatHelper.h"
#include "MQTTNetwork.h"
#include <thread>
#include <mutex>
#include <HTTPClient.h>
#include <Update.h>
#include "rtos/Mail.h"
#include "rtos/cmsis_os.h"
#include <app/ColorSensor/ColorSensor.h>
#include <platform/mbed.h>
#include <platform/Callback.h>
#include <RGBCollector.h>
#include <ColorConverter.h>

using namespace platform_debug;

namespace web_server
{
    struct mail_t{
       // uint32_t counter=0;   
         uint32_t message;
    };
}



class ESPWebServer //: private mbed::NonCopyable<ESPWebServer>
{
public:
    ESPWebServer();
    ~ ESPWebServer();
    void startup();
    void run_web_server();
    void notFound(AsyncWebServerRequest *request);
    void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) ;
    void post_mail(int message);

    void delegateMethodWebSocketClientPostEvent(const String& message, const String& event, uint32_t id=0, uint32_t reconnect=0);
    bool isRunning(){
       // std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
       // lck.lock();
        return running;
    }
    //using CallbackFun = std::function<void()>;
    void runCallbackPostMailToCollector(MeasEventType measEventType,uint32_t id){
        if(this->_callback!=nullptr){
            this->_callback(measEventType,id);
        }
    }
    void setCallbackPostMailToCollector(mbed::Callback<void(MeasEventType,uint32_t)> callback){
        this->_callback=callback;
    }

    void delegateMethodWebSocketClientText(uint32_t id,const String& text){
        _wss.client(id)->text(text);
    }
private:
    std::mutex _mtx;
    std::mutex _mtxEventSource;

    AsyncWebServer* _server;
    AsyncEventSource _events;
    AsyncWebSocket _wss;
    AsyncCallbackJsonWebHandler* _handler;
    bool running;
  
    rtos::Mail<web_server::mail_t, 16> _mail_box;
    mbed::Callback<void(MeasEventType,uint32_t)> _callback;
    std::thread _thread;
    
};
#endif