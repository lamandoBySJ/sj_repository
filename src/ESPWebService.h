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
#include <platform/mbed.h>
#include <platform/Callback.h>
#include <ColorCollector.h>
#include <rtos/Mutex.h>
#include "rtos/Thread.h"
#include "rtos/Mail.h"
#include "rtos/cmsis_os.h"
#include "rtos/ThisThread.h"
#include <DNSServer.h>

using namespace platform_debug;

namespace web_server
{    
    
    struct mail_t{
       // uint32_t counter=0;   
         uint32_t message;
    };
}
using namespace std;
using namespace web_server;

class alignas(4) [[gnu::may_alias]] ESPWebService 
{
public:
    ESPWebService():_thread(osPriorityNormal,1024*8),_mtx(),
        _dnsServer(nullptr),_server(nullptr),_events(nullptr),_wss(nullptr),
        _handler(nullptr),
        running(false)
    {
         _events=new AsyncEventSource("/events");
         _wss =new AsyncWebSocket("/ws");
        _server =new AsyncWebServer(80);
        _dnsServer=new DNSServer();
    }
    ~ESPWebService(){
        if(_handler){
            delete _handler;
        }
    }

    void startup();
    void shutdown();
    void run_web_service();
    void notFound(AsyncWebServerRequest *request);
    void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) ;
    bool isRunning();

    void setCallbackPostMailToCollector(mbed::Callback<void(MeasEventType,AsyncWebSocketClient*)> callback)
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_callback= callback;
    }
    void runCallbackPostMailToCollector(MeasEventType measEventType,AsyncWebSocketClient *client){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_callback(measEventType,client);
    }
    void delegateMethodWebSocketClientText(AsyncWebSocketClient *client,const String& text);
    void delegateMethodWebSocketClientEvent(const String& message, const String& event, uint32_t id, uint32_t reconnect);

    static ESPWebService* getESPWebService(){
        static ESPWebService* ESPwebService=new ESPWebService();
        return ESPwebService;
    }
private:
    rtos::Thread _thread;
    rtos::Mutex _mtx;
    DNSServer* _dnsServer;
    AsyncWebServer* _server;
    AsyncEventSource* _events;
    AsyncWebSocket* _wss;
    AsyncCallbackJsonWebHandler* _handler;
    AsyncWebSocketClient* _client;
    const char* PARAM_MESSAGE = "message";
    bool running;

    alignas(128) mbed::Callback<void(MeasEventType,AsyncWebSocketClient*)> _callback;
};

#endif