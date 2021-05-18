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
#include <ColorCollector.h>
#include <ColorConverter.h>
#include <rtos/Mutex.h>
#include "rtos/Thread.h"
#include "rtos/Mail.h"
#include "rtos/cmsis_os.h"
#include "rtos/ThisThread.h"
using namespace platform_debug;

namespace web_server
{    
    struct WebProperties
    {   
        WebProperties(){
            ap_ssid="STLB_SSID";
            ap_pass="Aa000000";
            http_user="admin";
            http_pass="admin";
            server_upload_url = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
        }
        String ap_ssid;
        String ap_pass;
        String http_user;
        String http_pass;
        String server_upload_url;

        WebProperties& operator=(const WebProperties& properties){
            this->ap_ssid = properties.ap_ssid;
            this->ap_pass = properties.ap_pass;
            this->http_user = properties.http_user;
            this->http_pass = properties.http_pass;
            this->server_upload_url = properties.server_upload_url;
            return *this;
        }
    };
    struct mail_t{
       // uint32_t counter=0;   
         uint32_t message;
    };
}
using namespace std;
using namespace web_server;
class ESPWebService 
{
public:

    static WebProperties webProperties;

    ESPWebService():_thread(osPriorityNormal,1024*6),_mtx(),
        _events("/events"),_wss("/ws"),_handler(nullptr),
        running(false),
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
    void run_web_service();
    void notFound(AsyncWebServerRequest *request);
    void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) ;

   // void delegateMethodWebSocketClientPostEvent(const String& message, const String& event, uint32_t id=0, uint32_t reconnect=0);
    bool isRunning(){
       // std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
       // lck.lock();
        std::lock_guard<rtos::Mutex> lck(_mtx);
        return running;
    }
    void runCallbackPostMailToCollector(MeasEventType measEventType,AsyncWebSocketClient *client){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        if(this->_callback!=nullptr){
            this->_callback(measEventType,client);
        }
    }
   
    void setCallbackPostMailToCollector(mbed::Callback<void(MeasEventType,AsyncWebSocketClient *client)> callback){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        this->_callback=callback;
    }

    void delegateMethodWebSocketClientText(AsyncWebSocketClient *client,const String& text){
        client->text(text);
    }
    void delegateMethodWebSocketClientEvent(const String& message, const String& event, uint32_t id, uint32_t reconnect){
        _events.send(message.c_str(),event.c_str(),id, reconnect);
    }

private:
    rtos::Thread _thread;
    rtos::Mutex _mtx;
    AsyncEventSource _events;
    AsyncWebSocket _wss;
    AsyncCallbackJsonWebHandler* _handler;
    AsyncWebSocketClient* _client;
    const char* PARAM_MESSAGE = "message";
    bool running;
    mbed::Callback<void(MeasEventType,AsyncWebSocketClient*)> _callback;

};

#endif
