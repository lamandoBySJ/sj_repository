#include "LoRaGateway.h"
using namespace platform_debug;
void LoRaGateway::startup()
{
    _topicCommandResponse = Platform::deviceInfo.Family+ String("/command/response/GW");
    _topicCommandRequest = Platform::deviceInfo.Family+ String("/command/request/GW");
    _topicTimeout = Platform::deviceInfo.Family+String("/send_timeout/#");

    _topics.push_back(_topicCommandRequest);
    _topics.push_back(_topicTimeout);
    //_threadMqttService.start(callback(this,&LoRaGateway::run_mqtt_service));
    //_threadLoraService.start(callback(this,&LoRaGateway::run_lora_service));
    _threadMqttService =std::thread(&LoRaGateway::run_mqtt_service,this);
    _threadLoraService=std::thread(&LoRaGateway::run_lora_service,this);
}
void LoRaGateway::run_mqtt_service()
{
    
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_message_t *mail = (mqtt::mail_message_t *)evt.value.p;
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
                                    _loRaNetwork.sendMessage(tagID,platform_debug::Platform::deviceInfo.BoardID,"{\"cmd\":\"learn\"}"); 
                                }else if( cmd == "track"){
                                    _mode="track";     
                                    _loRaNetwork.sendMessage(tagID,platform_debug::Platform::deviceInfo.BoardID,"{\"cmd\":\"track\"}"); 
                                }else if( cmd == "ON"){
                                    _loRaNetwork.sendMessage(tagID,platform_debug::Platform::deviceInfo.BoardID,"{\"cmd\":\"ON\"}");
                                }else if( cmd == "OFF"){
                                    _loRaNetwork.sendMessage(tagID,platform_debug::Platform::deviceInfo.BoardID,"{\"cmd\":\"OFF\"}");
                                }
                           // }    
                    }  
                    _mqttNetwork.publish(_topicCommandResponse,"{\""+Platform::deviceInfo.BoardID+"\":\"OK\"}");         
                }else if(_topicSplit.size()== 3){
                     _loRaNetwork.sendMessage(_topicSplit[2],platform_debug::Platform::deviceInfo.BoardID,"{\"cmd\":\""+_IPSProtocol.mode+"\"}");
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
    mqtt::mail_message_t *mail =  _mail_box_mqtt.alloc();
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
            if(mail->receiver == platform_debug::Platform::deviceInfo.BoardID){
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





