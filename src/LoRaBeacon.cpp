#include "LoRaBeacon.h"
using namespace platform_debug;

void LoRaBeacon::startup()
{
    
    _topicCommand = DeviceInfo::Family+ String("/command/request/BCN");
    _topics.push_back(_topicCommand);
    //_threadMqttService.start(callback(this,&LoRaBeacon::run_mqtt_service));
   // _threadLoraService.start(callback(this,&LoRaBeacon::run_lora_service));
   _threadMqttService = std::thread(&LoRaBeacon::run_mqtt_service,this);
   _threadLoraService = std::thread(&LoRaBeacon::run_lora_service,this);
}
void LoRaBeacon::run_mqtt_service()
{
    DynamicJsonDocument  doc(1024);
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;
            platform_debug::TracePrinter::printTrace("[BCN]MQTT:"+mail->topic);
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                if (doc.containsKey("beaconID")) {
                    platform_debug::TracePrinter::printTrace(doc["beaconID"].as<String>());
                } 
            }else{
                platform_debug::TracePrinter::printTrace(String("[BCN]MQTT: JsonParse ERROR..."));
            }
            
            _mail_box_mqtt.free(mail); 
        }
    }
}

void LoRaBeacon::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("[BCN]MQTT:onMqttConnectCallback");
}
void LoRaBeacon::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("[BCN]MQTT:onMqttDisconnectCallback");
}
void LoRaBeacon::onMessageMqttCallback(const String& topic,const String& payload)
{

        mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
        if(mail!=NULL){
            mail->topic = topic;
            mail->payload = payload;
            _mail_box_mqtt.put(mail) ;
        }
}


void LoRaBeacon::run_lora_service()
{
    while(true){
        osEvent evt= _mail_box_lora.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            platform_debug::TracePrinter::printTrace("[BCN]lora:Rx:"+mail->receiver+":Tx:"+mail->sender+"Packet:"+mail->packet);
            if(mail->receiver == platform_debug::DeviceInfo::BoardID){
                DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                if (!error)
                { 
                    platform_debug::TracePrinter::printTrace("[BCN]lora:Rx:"+doc.as<String>());
                }else{
                    platform_debug::TracePrinter::printTrace("[BCN]lora:Rx: JsonParse ERROR...");
                }
            }else if(mail->receiver == String("FAFA")){
                DynamicJsonDocument endNodeDoc(1024);
                endNodeDoc[mail->sender]= mail->rssi; 
                _mqttNetwork.publish("k49a/send_rssi/"+DeviceInfo::BoardID,endNodeDoc.as<String>());
                endNodeDoc.clear();
                platform_debug::TracePrinter::printTrace("[BCN]lora:Rx:PUB:send_rssi:beaconID:"+
                    platform_debug::DeviceInfo::BoardID+
                    ",tagID:"+mail->sender+
                    ",rssi:"+mail->rssi);
            }else {
                platform_debug::TracePrinter::printTrace("[BCN]lora:Rx:[x]:n/a");
            }
            
            
            _mail_box_lora.free(mail); 
        }
    }
}
void LoRaBeacon::onMessageLoRaCallback(const lora::mail_t& lora_mail)
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





