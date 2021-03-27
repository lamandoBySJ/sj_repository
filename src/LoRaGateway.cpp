#include "LoRaGateway.h"

void LoRaGateway::startup()
{
    _topicCommandRequest = platform_debug::DeviceInfo::Family+ String("/Command/Request");
    _topicSendFingerprints = platform_debug::DeviceInfo::Family+String("/send_fingerprint");
    _topics.push_back(_topicCommandRequest);
    _topics.push_back(_topicSendFingerprints);
    _threadMqttService.start(callback(this,&LoRaGateway::run_mqtt_service));
    _threadLoraService.start(callback(this,&LoRaGateway::run_lora_service));
}
void LoRaGateway::run_mqtt_service()
{
    
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;

            DynamicJsonDocument  doc(mail->payload.length()+128);
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                platform_debug::TracePrinter::printTrace(mail->topic);
                if(mail->topic == _topicSendFingerprints){
                    for(JsonPair p : doc.as<JsonObject>()){
                        String tagID = p.key().c_str();
                        JsonArray array = p.value().as<JsonArray>();
                        
                        if(array.size()!=_mapSetupBeacons.size()){
                            if(_mapRetry.find(tagID)==_mapRetry.end()){
                                _mapRetry[tagID]=0;
                                //_loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"LT\"}");
                            }else{
                                _mapRetry[tagID]++;
                               // _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"LT\"}");
                            }
                            if(_mapRetry[tagID]<2){
                                _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"LT\"}");
                            }else{
                                _mapRetry.erase(tagID);
                                _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"OFF\"}");
                                 platform_debug::TracePrinter::printTrace(String("[~]LoRa GW:Retry:timeout:eps sleep..."));
                            }
                        }else{
                            if(_mapRetry.find(tagID) !=_mapRetry.end()){
                                _mapRetry.erase(tagID);
                            }
                             _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"OFF\"}");
                             platform_debug::TracePrinter::printTrace(String("[~]LoRa GW:OK:DataSent:eps sleep..."));
                        }
                    }
                }else{
                    if(doc.containsKey("tagID")){
                        if(doc.containsKey("cmd")){
                            if(doc["cmd"].as<String>() == "LT"){
                                _loRaNetwork.sendMessage(doc["tagID"],platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"LT\"}");
                            }else if(doc["cmd"].as<String>() == "ON"){
                                _loRaNetwork.sendMessage(doc["tagID"].as<String>(),platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"ON\"}");
                            }else if(doc["cmd"].as<String>() == "OFF"){
                                _loRaNetwork.sendMessage(doc["tagID"].as<String>(),platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"OFF\"}");
                            }
                        }
                    }
                }
            }else{
                platform_debug::TracePrinter::printTrace(String("mqtt_service: JsonParse ERROR..."));
            }
            _mail_box_mqtt.free(mail); 
        }
    }
}


void LoRaGateway::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("LoRa GW Start Running");

}
void LoRaGateway::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("LoRa Gateway Terminaled");
}
void LoRaGateway::onMessageMqttCallback(const String& topic,const String& payload)
{
    if (std::find(_topics.begin(), _topics.end(), topic)!=_topics.end())
    {
        mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
        if(mail!=NULL){
            mail->topic = String(topic);
            mail->payload = String(payload);
            _mail_box_mqtt.put(mail) ;
        }
    }
}


void LoRaGateway::run_lora_service()
{
    
    while(true){
        osEvent evt= _mail_box_lora.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            if(mail->receiver == platform_debug::DeviceInfo::BoardID){
                 DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                if (!error)
                { 
                    platform_debug::TracePrinter::printTrace(mail->sender+String(":")+String(mail->rssi,DEC));
                    if (doc.containsKey("beacon")) {
                        platform_debug::TracePrinter::printTrace(doc["beacon"].as<String>());
                    } 
                }else{
                    platform_debug::TracePrinter::printTrace("lora GW: JsonParse ERROR...");
                }
                
            }else if(mail->receiver == String("FAFA")){
                platform_debug::TracePrinter::printTrace("lora GW:FAFA:sender:"+mail->sender);
            }
            else{
                platform_debug::TracePrinter::printTrace("[NA]lora GW: this msg not for me:sender:"+mail->sender);
            }
            
            
            _mail_box_lora.free(mail); 
        }
    }
}
void LoRaGateway::onMessageLoRaCallback(const lora::mail_t& lora_mail)
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





