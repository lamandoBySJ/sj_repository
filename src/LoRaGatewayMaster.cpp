#include "LoRaGatewayMaster.h"

void LoRaGatewayMaster::startup()
{
    _threadMqttService.start(callback(this,&LoRaGatewayMaster::run_mqtt_service));
    _threadLoraService.start(callback(this,&LoRaGatewayMaster::run_lora_service));
}
void LoRaGatewayMaster::run_mqtt_service()
{
    DynamicJsonDocument  doc(1024);
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                if (doc.containsKey("beacon")) {
                    platform_debug::TracePrinter::printTrace(doc["beacon"].as<String>());
                } 
            }else{
                platform_debug::TracePrinter::printTrace(String("mqtt_service: JsonParse ERROR..."));
            }
            
            _mail_box_mqtt.free(mail); 
        }
    }
}

void LoRaGatewayMaster::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("LoRa Gateway Master Runnning...");
}
void LoRaGatewayMaster::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("LoRa Gateway Master Abort...");
}
void LoRaGatewayMaster::onMessageMqttCallback(const String& topic,const String& payload)
{
      mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
      if(mail!=NULL){
        mail->topic = String(topic);
        mail->payload = String(payload);
        _mail_box_mqtt.put(mail) ;
      }
}


void LoRaGatewayMaster::run_lora_service()
{
    DynamicJsonDocument  doc(1024);
    while(true){
        osEvent evt= _mail_box_lora.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            DeserializationError error = deserializeJson(doc,mail->packet);
            if (!error)
            { 
                platform_debug::TracePrinter::printTrace(mail->sender+String(":")+String(mail->rssi,DEC));
                if (doc.containsKey("beacon")) {
                    platform_debug::TracePrinter::printTrace(doc["beacon"].as<String>());
                } 
            }else{
                platform_debug::TracePrinter::printTrace("lora Service: JsonParse ERROR...");
            }
            
            _mail_box_lora.free(mail); 
        }
    }
}
void LoRaGatewayMaster::onMessageLoRaCallback(const String& sender,const int& rssi,const String& packet)
{
      lora::mail_t *mail =  _mail_box_lora.alloc();
      if(mail!=NULL){
        mail->rssi = rssi;
        mail->sender = String(sender);
        mail->packet = String(packet);
        _mail_box_lora.put(mail) ;
      }
}





