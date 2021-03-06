#include "LoRaGateway.h"
using namespace platform_debug;
void LoRaGateway::startup()
{
    _topicCommandResponse = DeviceInfo::Family+ String("/command/response/GW");
    _topicCommandRequest = DeviceInfo::Family+ String("/command/request/GW");
   // _topicTimeout = DeviceInfo::Family+String("/send_timeout/#");

    _topics.push_back(_topicCommandRequest);
    //_topics.push_back(_topicTimeout);
    _threadMqttService.start(callback(this,&LoRaGateway::run_mqtt_service));
    _threadLoraService.start(callback(this,&LoRaGateway::run_lora_service));
}
void LoRaGateway::run_mqtt_service()
{
    
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;
            platform_debug::TracePrinter::printTrace("[GW]MQTT:"+mail->topic);
            DynamicJsonDocument  doc(mail->payload.length()+1024);
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                _topicSplit.clear();
                StringHelper::split( _topicSplit,mail->topic.c_str(),"/");
                if(_topicSplit.size()== 4){

                    if(doc.containsKey("beacons")){
                        _mapSetupBeacons.clear();
                        for(auto v :doc["beacons"].as<JsonArray>()){
                            for(JsonPair p : v.as<JsonObject>()){
                                _mapSetupBeacons[p.key().c_str()]=p.value().as<String>();
                                platform_debug::TracePrinter::printTrace("[GW]MQTT:beaconID:"+String(p.key().c_str()));
                            }
                        }
                        if(FFatHelper::writeFile(FFat,beacon_properties::path,doc.as<String>())){
                            platform_debug::TracePrinter::printTrace("[GW]MQTT:W:beacons:OK");
                        }else{
                            platform_debug::TracePrinter::printTrace("[GW]MQTT:W:beacons:ERROR");
                        }
                    }else if(doc.containsKey("tags")){
                        _mapTagLocation.clear();
                        for(auto v :doc["tags"].as<JsonArray>()){
                            for(JsonPair p : v.as<JsonObject>()){
                                _mapTagLocation[p.key().c_str()]=p.value().as<String>();
                                platform_debug::TracePrinter::printTrace("[GW]MQTT:tagID:"+String(p.key().c_str()));
                            } 
                        }
                         
                    }else if(doc.containsKey("technology")){
                        _IPSProtocol.technology = doc["technology"].as<String>();
                        _IPSProtocol.family     = doc["family"].as<String>();
                        _IPSProtocol.gateway    = doc["gateway"].as<String>();
                        _IPSProtocol.collector  = doc["collector"].as<String>();
                        _IPSProtocol.mode       = doc["mode"].as<String>();
                           
                    }else if(doc.containsKey("cmd")){
                        
                            // if(doc.containsKey("cmd")&&doc.containsKey("tagID")){
                                String tagID = doc["tagID"].as<String>();
                                String cmd = doc["cmd"].as<String>();
                                if( cmd== "learn"){
                                    _mode="learn"; 
                                    _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"learn\"}"); 
                                }else if( cmd == "track"){
                                    _mode="track";     
                                    _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"track\"}"); 
                                }else if( cmd == "ON"){
                                    _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"ON\"}");
                                }else if( cmd == "OFF"){
                                    _loRaNetwork.sendMessage(tagID,platform_debug::DeviceInfo::BoardID,"{\"cmd\":\"OFF\"}");
                                }
                           // }    
                    }  
                    _mqttNetwork.publish(_topicCommandResponse,"{\""+DeviceInfo::BoardID+"\":\"OK\"}");         
                }else if(_topicSplit.size()== 3){
                     _loRaNetwork.sendMessage(_topicSplit[2],platform_debug::DeviceInfo::BoardID,"{\"cmd\":\""+_IPSProtocol.mode+"\"}");
                     platform_debug::TracePrinter::printTrace(String("[GW]:MQTT:[send_timeout]:"+mail->payload)); 
                }else{
                    platform_debug::TracePrinter::printTrace(String("[GW]:MQTT:[x]:"+mail->topic));
                }
            }else{
                platform_debug::TracePrinter::printTrace(String("[GW]:MQTT:[x]:JsonParse ERROR..."));
            }
            _mail_box_mqtt.free(mail); 
        }
    }
}


void LoRaGateway::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("[GW]MQTT::onMqttConnectCallback");

}
void LoRaGateway::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("[GW]MQTT:onMqttDisconnectCallback");
}
void LoRaGateway::onMessageMqttCallback(const String& topic,const String& payload)
{
   /* _topicMatch.clear();
    StringHelper::split(_topicMatch,topic.c_str(),"/");
    if(_topicMatch.size() == 4 && _topicMatch[3]=="GW"){
        mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
        if(mail!=NULL){
            mail->topic = topic;
            mail->payload = payload;
            _mail_box_mqtt.put(mail) ;
        }
    }else if(_topicMatch.size() == 3 && _topicMatch[1]=="send_timeout"){
        mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
        if(mail!=NULL){
            mail->topic = topic;
            mail->payload = payload;
            _mail_box_mqtt.put(mail) ;
        }
    }*/
    mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
        if(mail!=NULL){
            mail->topic = topic;
            mail->payload = payload;
            _mail_box_mqtt.put(mail) ;
        }
}


void LoRaGateway::run_lora_service()
{
    
    while(true){
        osEvent evt= _mail_box_lora.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            platform_debug::TracePrinter::printTrace("[GW]lora:Rx:"+mail->receiver+String(":Tx:")+mail->sender);
            if(mail->receiver == platform_debug::DeviceInfo::BoardID){
                 DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                platform_debug::TracePrinter::printTrace(mail->packet);
                if (!error)
                { 
                    platform_debug::TracePrinter::printTrace("[#]lora GW: JsonParse OK!");
                }else{
                    platform_debug::TracePrinter::printTrace("[x]lora GW: JsonParse ERROR...");
                }
                
            }else if(mail->receiver == String("FAFA")){
                platform_debug::TracePrinter::printTrace("[GW]lora:Rx:FAFA:n/a");
            }
            else{
                platform_debug::TracePrinter::printTrace("[GW]lora:Rx:[x]:n/a");
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





