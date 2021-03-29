#include "LoRaBeacon.h"

void LoRaBeacon::startup()
{
    
    _topicCommand = DeviceInfo::Family+ String("/command/request/")+DeviceInfo::BoardID;
      _topics.push_back(_topicCommand);
    _threadMqttService.start(callback(this,&LoRaBeacon::run_mqtt_service));
    _threadLoraService.start(callback(this,&LoRaBeacon::run_lora_service));
}
void LoRaBeacon::run_mqtt_service()
{
    DynamicJsonDocument  doc(1024);
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                if (doc.containsKey("beaconID")) {
                    platform_debug::TracePrinter::printTrace(doc["beaconID"].as<String>());
                } 
            }else{
                platform_debug::TracePrinter::printTrace(String("[x]LoRA BCN: JsonParse ERROR..."));
            }
            
            _mail_box_mqtt.free(mail); 
        }
    }
}

void LoRaBeacon::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("LoRa BCN:onMqttConnectCallback");
}
void LoRaBeacon::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("LoRa BCN:onMqttDisconnectCallback");
}
void LoRaBeacon::onMessageMqttCallback(const String& topic,const String& payload)
{
    if (std::find(_topics.begin(), _topics.end(), topic)!=_topics.end())
    {
        mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
        if(mail!=NULL){
            mail->topic = topic;
            mail->payload = payload;
            _mail_box_mqtt.put(mail) ;
        }
    }
}


void LoRaBeacon::run_lora_service()
{
    
    DynamicJsonDocument endNodeDoc(1024);
    
    while(true){
        osEvent evt= _mail_box_lora.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            if(mail->receiver == platform_debug::DeviceInfo::BoardID){
                platform_debug::TracePrinter::printTrace("lora BCN:receiver:"+mail->receiver+String(",sender:")+mail->sender);
                DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                if (!error)
                { 
                    platform_debug::TracePrinter::printTrace(mail->sender+String(":")+String(mail->rssi,DEC));
                    if (doc.containsKey("beacon")) {
                        platform_debug::TracePrinter::printTrace(doc["beacon"].as<String>());
                    } 
                }else{
                    platform_debug::TracePrinter::printTrace("lora BCN: JsonParse ERROR...");
                }
            }else if(mail->receiver == String("FAFA")){
                platform_debug::TracePrinter::printTrace("lora BCN:FAFA:sender:"+mail->sender);
                
                endNodeDoc["tagID"]=mail->sender;
                endNodeDoc["rssi"]= mail->rssi;
                endNodeDoc["beaconID"] = platform_debug::DeviceInfo::BoardID;
                _mqttNetwork.publish("k49a/send_rssi",endNodeDoc.as<String>());
                endNodeDoc.clear();

            }else {
                platform_debug::TracePrinter::printTrace("[NA]lora BCN: this msg not for me:receiver:"+mail->receiver+String(",sender:")+mail->sender);
                platform_debug::TracePrinter::printTrace("[NA]packet:"+mail->packet);
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





