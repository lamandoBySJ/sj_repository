#include "LoRaGateway.h"
using namespace platform_debug;

void LoRaGateway::startup()
{
    _topicCommandResponse = DeviceInfo::Family+ String("/command/response/GW");
    _topicCommandRequest = DeviceInfo::Family+ String("/command/request/GW");
    _topicSendFingerprints = DeviceInfo::Family+String("/send_fingerprint");

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
            platform_debug::TracePrinter::printTrace(mail->topic);
            DynamicJsonDocument  doc(mail->payload.length()+1024);
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                if(mail->topic == _topicSendFingerprints){

                    
                    for(JsonPair p : doc.as<JsonObject>()){
                        String tagID = p.key().c_str();
                        JsonArray array = p.value().as<JsonArray>();
                        
                        if(array.size()!=_mapSetupBeacons.size()){
                            if(_mapRetry.find(tagID)==_mapRetry.end()){
                                _mapRetry[tagID]=0;
                            }else{
                                _mapRetry[tagID]++;
                            }
                            if(_mapRetry[tagID]<2){
                                _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"LT\"}");
                            }else{
                                _mapRetry.erase(tagID);
                                _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"OFF\"}");
                                platform_debug::TracePrinter::printTrace(String("[x]LoRa GW:Retry:timeout:eps sleep..."));
                            }
                        }else{
                            if(_mapRetry.find(tagID) !=_mapRetry.end()){
                                _mapRetry.erase(tagID);
                            }
                            _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"OFF\"}");
                            platform_debug::TracePrinter::printTrace(String("[~]LoRa GW:OK:Sent:eps sleep..."));

                            if(_mode == "learn"){
                                _topicLT=DeviceInfo::Family+"/"+_mode+"/"+tagID+"/"+_mapTagLocation[tagID];
                            }else{
                                _topicLT=DeviceInfo::Family+"/"+_mode+"/"+tagID;
                            }
                            _payload="{";
                            for(JsonObject obj:array){
                                for(JsonPair p:obj){
                                    _payload+=p.key().c_str();
                                    _payload+=":";
                                    _payload+=p.value().as<int>();
                                    _payload+=",";
                                }
                            }
                            _payload+="}";
                            _mqttNetwork.publish(_topicLT,_payload);
                        }
                    }
                }else {
                    if(doc.containsKey("technology")){
                        if(doc["technology"].as<String>() == "LoRa"){
                            if(doc.containsKey("cmd")&&doc.containsKey("tagID")){
                                String tagID = doc["tagID"];
                                String cmd = doc["cmd"].as<String>();
                                if( cmd== "learn"){
                                    _mode="learn"; 
                                    _mapTagLocation[tagID] =  doc["location"].as<String>();
                                    _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"LT\"}"); 
                                }else if( cmd == "track"){
                                    _mode="track";     
                                    _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"LT\"}"); 
                                }else if( cmd == "ON"){
                                    _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"ON\"}");
                                }else if( cmd == "OFF"){
                                    _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"OFF\"}");
                                }
                            }else if(doc.containsKey("beacons")){
                                _mapSetupBeacons.clear();
                                for(auto v :doc["beacons"].as<JsonArray>()){
                                    for(JsonPair p : v.as<JsonObject>()){
                                        _mapSetupBeacons[p.key().c_str()]=p.value().as<String>();
                                        platform_debug::TracePrinter::printTrace("GW:beaconID:"+String(p.key().c_str()));
                                    }
                                }
                                _mqttNetwork.publish(_topicCommandResponse,"{\""+DeviceInfo::BoardID+"\":\"OK\"}");         
                            }
                        }                    
                    }    
                }
            }else{
                platform_debug::TracePrinter::printTrace(String("GW:mqtt_service: JsonParse ERROR..."));
                platform_debug::TracePrinter::printTrace(doc.as<String>());
            }
            _mail_box_mqtt.free(mail); 
        }
    }
}


void LoRaGateway::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("LoRa GW::onMqttConnectCallback");

}
void LoRaGateway::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("LoRa GW:onMqttDisconnectCallback");
}
void LoRaGateway::onMessageMqttCallback(const String& topic,const String& payload)
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





