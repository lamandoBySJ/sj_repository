#pragma once

#include "platform_debug.h"
#include <WiFiType.h>
#include <WiFi.h>

struct [[gnu::may_alias]] mail_wifi_event_t{
    system_event_id_t event_id;
    wifi_mode_t mode;
};

class WiFiService
{
public:
    //std::function<void(system_event_id_t event, system_event_info_t info)>  _wifiCB;
    explicit WiFiService():_wifi_channel(random(0,11))
    {
        //_wifiCB=std::bind(&NetworkService::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2);
        WiFi.onEvent(std::bind(&WiFiService::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2));
    }
    void startup(){
         osStatus status =  _threadWiFiEvent.start(mbed::callback(this,&WiFiService::task_system_event_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadWiFiEvent.get_name()):NULL);
    }
    void init();

    void switch_wifi_mode(wifi_mode_t mode,system_event_id_t event=SYSTEM_EVENT_STA_DISCONNECTED){
        mail_wifi_event_t* mail=_mailBoxWiFiEvent.alloc();
        if(mail){
            mail->mode = mode;
            mail->event_id=event;
            _mailBoxWiFiEvent.put(mail);
        }
    }

    void WiFiEvent(system_event_id_t event, system_event_info_t info)
    {
        _onWiFiEventCallback.call(event,info);
    }
    void task_system_event_service();
  
    int32_t getWiFiChannel(){
        return _wifi_channel;
    }
      /*
    void removeOnWiFiModeCallbacks(){
        _onWiFiEventCallbacks.clear();
    }
    void addOnWiFiEventCallback(mbed::Callback<void(system_event_id_t id,system_event_info_t)> func){
        _onWiFiEventCallbacks.push_back(func);
    }*/
    /*
    void delegateMethodWiFiEvent(system_event_id_t event, system_event_info_t info){
        mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
        if(evt!=NULL){
        evt->event_id=event;
        _mailBoxWiFiEvent.put(evt);
        }
    }*/

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
private:
    rtos::Mail<mail_wifi_event_t, 8> _mailBoxWiFiEvent;
    rtos::Thread _threadWiFiEvent;
    rtos::Mutex _mtx;
    mbed::Callback<void(system_event_id_t,system_event_info_t)> _onWiFiEventCallback;
    //std::vector<mbed::Callback<void(system_event_id_t,system_event_info_t)>>  _onWiFiEventCallbacks;
    std::vector<mbed::Callback<void(const mail_wifi_event_t&)>>  _onWiFiServiceCallbacks;
    int32_t _wifi_channel;
};