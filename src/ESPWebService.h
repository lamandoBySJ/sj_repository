#ifndef ESP_WEB_SERVICE_H
#define ESP_WEB_SERVICE_H

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
#include <mutex>

#include <ColorCollector.h>
#include <DNSServer.h>
#include <ColorConverter.h>

using namespace std;

class  ESPWebService
{
public:
    ESPWebService():_thread(osPriorityNormal,1024*8),
    _dnsServer(nullptr),_server(nullptr),_events(nullptr),_wss(nullptr),
        _handler(nullptr)
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
    void startup(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        if(_thread.get_state()!=Thread::Running){
            _thread.start(callback(this,&ESPWebService::run_web_service));
        }
    }
    void terminate(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        if(_thread.get_state()==Thread::Running){
            _thread.terminate();
        }
    }
    bool isRunning(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        return _thread.get_state()==Thread::Running;
    }
    void run_web_service();
    void notFound(AsyncWebServerRequest *request);
    void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) ;

    void addCallbackOnWsEvent(mbed::Callback<void(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType, void*, uint8_t*, size_t)> callback)
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_callbacks.push_back(callback);
    }

    void delegateMethodWebSocketClientText(AsyncWebSocketClient *client,const String& text);
    void delegateMethodWebSocketClientEvent(const String& message, const String& event, uint32_t id, uint32_t reconnect);
    void set_signal_wifi_mode(uint32_t flags=0)
    {
        _thread.flags_set(flags);
    }
 
private:

    rtos::Mutex _mtx;
    rtos::Thread _thread;
    DNSServer* _dnsServer;
    AsyncWebServer* _server;
    AsyncEventSource* _events;
    AsyncWebSocket* _wss;
    AsyncCallbackJsonWebHandler* _handler;
    AsyncWebSocketClient* _client;
    const char* PARAM_MESSAGE = "message";
    std::vector<mbed::Callback<void(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType, void*, uint8_t*, size_t)>> _callbacks;
    
    
};

#endif
