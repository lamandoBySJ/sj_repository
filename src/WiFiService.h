#pragma once

#include "platform_debug.h"
#include <WiFiType.h>
#include <WiFi.h>
#include <drivers/Timeout.h>


struct [[gnu::may_alias]] mail_wifi_event_t{
    system_event_id_t event_id;
};

class WiFiService
{
public:
    //std::function<void(system_event_id_t event, system_event_info_t info)>  _wifiCB;
    explicit WiFiService():_wifi_channel(random(0,11)),_connected(false)
    {
        //_wifiCB=std::bind(&NetworkService::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2);
       wifi_event_id = WiFi.onEvent(std::bind(&WiFiService::WiFiEvent,this,std::placeholders::_1));
    }
    void shutdown(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        _threadWiFiEvent.terminate();
    }
    void startup(){
         osStatus status =  _threadWiFiEvent.start(mbed::callback(this,&WiFiService::task_system_event_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadWiFiEvent.get_name()):NULL);
    }
    void switch_wifi_mode_to_AP(){
        //_mtx.lock();
        WiFi.removeEvent( wifi_event_id );
        WiFi.mode(WIFI_MODE_AP);
        WiFi.enableAP(true);
        /* we never unlock this mtx */
        //_mtx.unlock();
    }

    void WiFiEvent(system_event_id_t event)
    {
        mail_wifi_event_t* wifi_event=_mailBoxWiFiEvent.alloc();
        wifi_event->event_id=event;
        _mailBoxWiFiEvent.put(wifi_event);
    }

    void task_system_event_service();
  
    int32_t getWiFiChannel(){
        return _wifi_channel;
    }

    void cleanupCallbacks(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        _onWiFiServiceCallbacks.clear();
    }
    void addOnWiFiServiceCallback(mbed::Callback<void(const mail_wifi_event_t&)> func){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        _onWiFiServiceCallbacks.push_back(func);
    }
    void invokeOnWiFiServiceCallback(const mail_wifi_event_t& wifi_event){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        for(auto& v :_onWiFiServiceCallbacks){
            v.call(wifi_event);
        } 
    }
    bool isConnected(){
        std::lock_guard<rtos::Mutex> lck(_mtx_connected);
        return _connected;
    }
protected:
    void init();
private:
    rtos::Mail<mail_wifi_event_t, 8> _mailBoxWiFiEvent;
    rtos::Thread _threadWiFiEvent;
    rtos::Mutex _mtx;
    rtos::Mutex _mtx_connected;
   //mbed::Callback<void(system_event_id_t,system_event_info_t)> _onWiFiEventCallback;
    //std::vector<mbed::Callback<void(system_event_id_t,system_event_info_t)>>  _onWiFiEventCallbacks;
    std::vector<mbed::Callback<void(const mail_wifi_event_t&)>>  _onWiFiServiceCallbacks;
    
    wifi_event_id_t wifi_event_id;
    int32_t _wifi_channel;
    bool _connected;
};