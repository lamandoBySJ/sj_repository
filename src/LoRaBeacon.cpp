#include "LoRaBeacon.h"

namespace product_api
{
BeaconProperties& get_beacon_properties()
{
    static  BeaconProperties  beaconProperties;
    return beaconProperties;
}
}

void LoRaBeacon::startup()
{   
    _threadMqttService.start(mbed::callback(this,&LoRaBeacon::run_mqtt_service));
    _threadLoraService.start(mbed::callback(this,&LoRaBeacon::run_lora_service));
}
void LoRaBeacon::run_mqtt_service()
{
    DynamicJsonDocument  doc(1024);
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_on_message_t *mail = (mqtt::mail_on_message_t *)evt.value.p;
            TracePrinter::printTrace("[BCN]MQTT:"+mail->topic);
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                if (doc.containsKey("beaconID")) {
                    TracePrinter::printTrace(doc["beaconID"].as<String>());
                } 
            }else{
                TracePrinter::printTrace(String("[BCN]MQTT: JsonParse ERROR..."));
            }
            
            _mail_box_mqtt.free(mail); 
        }
    }
}
/*
void LoRaBeacon::onMqttConnectCallback(bool sessionPresent)
{
    TracePrinter::printTrace("[BCN]MQTT:onMqttConnectCallback");
}
void LoRaBeacon::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    TracePrinter::printTrace("[BCN]MQTT:onMqttDisconnectCallback");
}*/
void LoRaBeacon::run_lora_service()
{
    while(true){
        osEvent evt= _mail_box_lora.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            TracePrinter::printTrace("[BCN]lora:Rx:"+mail->receiver+":Tx:"+mail->sender+"Packet:"+mail->packet);
            if(mail->receiver == platformio_api::get_device_info().BoardID){
                DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                if (!error)
                { 
                    TracePrinter::printTrace("[BCN]lora:Rx:"+doc.as<String>());
                }else{
                    TracePrinter::printTrace("[BCN]lora:Rx: JsonParse ERROR...");
                }
            }else if(mail->receiver == String("FAFA")){
                DynamicJsonDocument endNodeDoc(1024);
                endNodeDoc[mail->sender]= mail->rssi; 
                _asyncMqttClientService.publish("k49a/send_rssi/"+platformio_api::get_device_info().BoardID,endNodeDoc.as<String>());
                endNodeDoc.clear();
                TracePrinter::printTrace("[BCN]lora:Rx:PUB:send_rssi:beaconID:"+
                    platformio_api::get_device_info().BoardID+
                    ",tagID:"+mail->sender+
                    ",rssi:"+mail->rssi);
            }else {
                TracePrinter::printTrace("[BCN]lora:Rx:[x]:n/a");
            }
            
            
            _mail_box_lora.free(mail); 
        }
    }
}
void LoRaBeacon::onLoRaMessageCallback(const lora::mail_t& lora_mail)
{
      lora::mail_t *mail =  _mail_box_lora.alloc();
      if(mail!=NULL){
        mail->rssi = lora_mail.rssi;
        mail->sender = lora_mail.sender;
        mail->receiver = lora_mail.receiver;
        mail->packet = lora_mail.packet;
        _mail_box_lora.put(mail) ;
      }
}





