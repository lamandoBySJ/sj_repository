#include "WiFiService.h"

//const char* MQTT_HOST="mslmqtt.mic.com.cn";
const char* WIFI_SSID="Mitac IOT_GPS";
const char* WIFI_PASSWORD="6789067890";

void WiFiService::init(){
    WIFI_SSID    = platformio_api::get_user_properties().ssid.c_str();
    WIFI_PASSWORD= platformio_api::get_user_properties().pass.c_str();
    _mtx.lock();
    WiFi.setAutoReconnect(false);
    WiFi.setAutoConnect(false);
    WiFi.mode(WIFI_OFF);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD,_wifi_channel);
    _mtx.unlock();
}

void WiFiService::task_system_event_service(){
   
    init();
    mail_wifi_event_t wifi_event;
    while(true){
        osEvent evt = _mailBoxWiFiEvent.get();
        if (evt.status == osEventMail) {
            mail_wifi_event_t* event = ( mail_wifi_event_t *)evt.value.p;
            TracePrinter::printTrace("[WiFi-event] event: "+String(event->event_id,DEC));
            if(_mtx.trylock()){
                switch(event->event_id) {
                    case SYSTEM_EVENT_STA_STOP:
                        WiFi.mode(WIFI_STA);
                        WiFi.begin(WIFI_SSID, WIFI_PASSWORD,_wifi_channel);
                        TracePrinter::printTrace("WiFi Connecting...");
                    break;
                    case SYSTEM_EVENT_STA_START:
                    break;
                    case SYSTEM_EVENT_STA_GOT_IP:
                        TracePrinter::printTrace("WiFi connected:SYSTEM_EVENT_STA_GOT_IP,IP address: "+WiFi.localIP().toString());
                        _mtx_connected.lock();
                        _connected =true;
                        _mtx_connected.unlock();
                        break;
                    case SYSTEM_EVENT_STA_DISCONNECTED:
                        TracePrinter::printTrace("WiFi lost connection"); 
                        _mtx_connected.lock();
                        _connected =false;
                        _mtx_connected.unlock();
                        WiFi.mode(WIFI_OFF);
                        break;
                    case SYSTEM_EVENT_STA_CONNECTED:
                        TracePrinter::printTrace("WiFi connected:SYSTEM_EVENT_STA_CONNECTED"); 
                        break;
                    default:
                    TracePrinter::printTrace("WiFi Event Unknow"); 
                    break;
                }
                _mtx.unlock();
            }
            wifi_event.event_id=event->event_id;
            invokeOnWiFiServiceCallback(wifi_event);
            _mailBoxWiFiEvent.free(event);
        }
    }
}



