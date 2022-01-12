#include "WiFiService.h"

//const char* MQTT_HOST="mslmqtt.mic.com.cn";

void WiFiService::eventLooper(){
   

    WiFi.setAutoReconnect(false);
    WiFi.setAutoConnect(false);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid,_pass);

   while(true){
        osEvent evt = _mailBoxWiFiEvent.get();
        if (evt.status == osEventMail) {
            mail_wifi_event_t* mail = ( mail_wifi_event_t *)evt.value.p;
           
            TracePrinter::printTrace("[Mail] WiFi-event: "+String(mail->event_id,DEC));
            if(mail->mode==WIFI_MODE_STA){
                switch(mail->event_id) {
                    case SYSTEM_EVENT_STA_STOP:
                        WiFi.mode(WIFI_STA);
                        WiFi.begin(_ssid,_pass);
                        TracePrinter::printTrace("*SYSTEM_EVENT_STA_STOP");
                    break;
                    case SYSTEM_EVENT_STA_START:
                        TracePrinter::printTrace("*SYSTEM_EVENT_STA_START"); 
                        TimeoutManager::countdown(this);

                    break;
                    case SYSTEM_EVENT_STA_GOT_IP:
                        TracePrinter::printTrace("*SYSTEM_EVENT_STA_GOT_IP->IP address: "+WiFi.localIP().toString());
 
                        TimeoutManager::remove(this);
                         _connected=true;
                        break;
                    case SYSTEM_EVENT_STA_DISCONNECTED:
                        TracePrinter::printTrace("*SYSTEM_EVENT_STA_DISCONNECTED"); 
                         _connected=false;
                        WiFiEvent(SYSTEM_EVENT_STA_STOP);
                        break;
                    case SYSTEM_EVENT_STA_CONNECTED:
                        TracePrinter::printTrace("*SYSTEM_EVENT_STA_CONNECTED"); 
                        break;
                    default:
                    TracePrinter::printTrace("[x] WiFi-Event-Unknow"); 
                    break;
                }
            }else{
                WiFi.mode(WIFI_OFF);
                WiFi.mode(WIFI_MODE_AP);
                WiFi.enableAP(true);
                WiFi.softAP(_ap_ssid,_ap_pass);
            }
            onWiFiEvent(mail->event_id);
            _mailBoxWiFiEvent.free(mail);
        }
    }
}



