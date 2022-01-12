#pragma once

#include "platform_debug.h"
#include <WiFiType.h>
#include <WiFi.h>
#include <drivers/Timeout.h>
#include <set>
#include <atomic>
#include "AsyncMqttClientService.h"
#include "Countdown.h"
#include "string.h"
struct  mail_wifi_event_t{
    wifi_mode_t mode;
    system_event_id_t event_id;
};
class WiFiEventHandler
{
public:
virtual  ~WiFiEventHandler()=default;
virtual void onWiFiEvent(const system_event_id_t& wifi_event)=0;
};

class WiFiEventListener:public WiFiEventHandler
{
public:
WiFiEventListener()=default;
virtual ~WiFiEventListener()=default;
virtual const char* listener()=0;
void onWiFiEvent(const system_event_id_t& wifi_event) override{
        switch(wifi_event){
        case SYSTEM_EVENT_STA_STOP:
            _wifi_event_got_ip=false;
            break;
        case SYSTEM_EVENT_STA_START:
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            _wifi_event_got_ip=true;
           
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            _wifi_event_got_ip=false;
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            break;
        default:
            break;
        }
        PlatformDebug::printf("[WiFi]: wifi_event:%d\n",wifi_event);
}

bool connected() const {
  return _wifi_event_got_ip;
}
private:

std::atomic<bool>  _wifi_event_got_ip;
};
class WiFiService :public WiFiEventHandler,public TimeoutCountdown
{
public:
    //std::function<void(system_event_id_t event, system_event_info_t info)>  _wifiCB;
    WiFiService():TimeoutCountdown(30,"wifi")
    {
        //_wifiCB=std::bind(&NetworkService::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2);
       _wifi_event_id = WiFi.onEvent(std::bind(&WiFiService::WiFiEvent,this,std::placeholders::_1));
    }
    void onTimeout() override{
        ESP.restart();
    }
    void onWiFiEventTimeout(){
        PlatformDebug::printf("ERROR:onWiFiEventTimeout:ESP.Restart in 3s.");
        ThisThread::sleep_for(3000);
        ESP.restart();
    }
    static void shutdown() {
        getInstance()._shutdown();
    }
    void _shutdown() {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        if(_threadWiFiEvent.get_state()==Thread::Running){
            _threadWiFiEvent.terminate();
        }
    }
    static void switchWifiMode(wifi_mode_t mode=WIFI_AP){
        getInstance()._switchWifiMode(mode);
    }
    void _switchWifiMode(wifi_mode_t mode=WIFI_AP,bool addEvent=false){

        WiFi.removeEvent( _wifi_event_id );

        if(addEvent){
            _wifi_event_id = WiFi.onEvent(std::bind(&WiFiService::WiFiEvent,this,std::placeholders::_1));
        }
        do{
            delay(100);
        }while(!_mailBoxWiFiEvent.empty());
 
        TimeoutManager::remove(this);

        mail_wifi_event_t* wifi_event=_mailBoxWiFiEvent.alloc();
        if(wifi_event){
            wifi_event->mode=mode;
            _mailBoxWiFiEvent.put(wifi_event);
        }
    }

    void onWiFiEvent(const system_event_id_t& wifi_event) override{
        for(auto& v :_eventListener){
            v->onWiFiEvent(wifi_event);
        }
    }

    void WiFiEvent(system_event_id_t event)
    {
        mail_wifi_event_t* mail=_mailBoxWiFiEvent.alloc();
        if(mail){
               mail->mode=WIFI_MODE_STA;
                mail->event_id=event;
                _mailBoxWiFiEvent.put_from_isr(mail);
        }
    }

    void eventLooper();
  
    void addWiFiEventListener(WiFiEventListener* listener){
        std::lock_guard<rtos::Mutex> lck(_mtx);
       _eventListener.insert(listener);
    }
    void init(const char* ssid,const char* pass){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        _ssid = ssid;
        _pass = pass;
        if(_threadWiFiEvent.get_state()!=Thread::Running){
             osStatus status =  _threadWiFiEvent.start(mbed::callback(this,&WiFiService::eventLooper));
            (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadWiFiEvent.get_name()):NULL);
        }
    }
    static WiFiService& getInstance(){
        static WiFiService* wifiService=new WiFiService();
        return *wifiService;
    }
    static void startup(const char* ssid,const char* pass,WiFiEventListener* listener){
        if(listener){
            getInstance().addWiFiEventListener(listener);
            getInstance().init(ssid,pass);
        }
    }
    static bool connected(){
        return getInstance()._connected.load();
    }
    static void setSoftAP(const char* ssid,const char* pass){
        getInstance()._setSoftAP(ssid,pass);
    }

    void  _setSoftAP(const char* ssid,const char* pass){
        strncpy(_ap_ssid,ssid,sizeof(_ap_ssid));
        strncpy(_ap_pass,pass,sizeof(_ap_pass));
    }
    
protected:

private:

    rtos::Mail<mail_wifi_event_t, 4> _mailBoxWiFiEvent;
    rtos::Thread _threadWiFiEvent;
    rtos::Mutex _mtx;
    std::set<WiFiEventListener*>  _eventListener;
    wifi_event_id_t _wifi_event_id;
    std::atomic<bool> _connected;
    const char* _ssid;
    const char* _pass;
    char _ap_ssid[128];
    char _ap_pass[128];
};