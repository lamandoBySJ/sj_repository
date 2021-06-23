#include "LoRaGateway.h"
namespace product_api
{
   
 IPSProtocol& get_ips_protocol()
 {
     static IPSProtocol ipsProtocol;
     return ipsProtocol;
}
}

void LoRaGateway::startup()
{
    _topicCommandResponse = platformio_api::get_device_info().Family+ String("/command/response/GW");
    _threadMqttService.start(mbed::callback(this,&LoRaGateway::run_mqtt_service));
    _threadLoraService.start(mbed::callback(this,&LoRaGateway::run_lora_service));
}
void LoRaGateway::run_mqtt_service()
{
    
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_on_message_t *mail = (mqtt::mail_on_message_t *)evt.value.p;
            TracePrinter::printTrace("[GW]MQTT:"+mail->topic);
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
                                TracePrinter::printTrace("[GW]MQTT:beaconID:"+String(p.key().c_str()));
                            }
                        }
                        if(FatHelper.writeFile(FFat,product_api::get_beacon_properties().path,doc.as<String>())){
                            TracePrinter::printTrace("[GW]MQTT:W:beacons:OK");
                        }else{
                            TracePrinter::printTrace("[GW]MQTT:W:beacons:ERROR");
                        }
                    }else if(doc.containsKey("tags")){
                        _mapTagLocation.clear();
                        for(auto v :doc["tags"].as<JsonArray>()){
                            for(JsonPair p : v.as<JsonObject>()){
                                _mapTagLocation[p.key().c_str()]=p.value().as<String>();
                                TracePrinter::printTrace("[GW]MQTT:tagID:"+String(p.key().c_str()));
                            } 
                        }
                         
                    }else if(doc.containsKey("technology")){
                       product_api::get_ips_protocol().technology = doc["technology"].as<String>();
                       product_api::get_ips_protocol().family     = doc["family"].as<String>();
                       product_api::get_ips_protocol().gateway    = doc["gateway"].as<String>();
                       product_api::get_ips_protocol().collector  = doc["collector"].as<String>();
                       product_api::get_ips_protocol().mode       = doc["mode"].as<String>();
                           
                    }else if(doc.containsKey("cmd")){
                        
                            // if(doc.containsKey("cmd")&&doc.containsKey("tagID")){
                                String tagID = doc["tagID"].as<String>();
                                String cmd = doc["cmd"].as<String>();
                                if( cmd== "learn"){
                                    _mode="learn"; 
                                    _loRaService.sendMessage(tagID,platformio_api::get_device_info().BoardID,"{\"cmd\":\"learn\"}"); 
                                }else if( cmd == "track"){
                                    _mode="track";     
                                    _loRaService.sendMessage(tagID,platformio_api::get_device_info().BoardID,"{\"cmd\":\"track\"}"); 
                                }else if( cmd == "ON"){
                                    _loRaService.sendMessage(tagID,platformio_api::get_device_info().BoardID,"{\"cmd\":\"ON\"}");
                                }else if( cmd == "OFF"){
                                    _loRaService.sendMessage(tagID,platformio_api::get_device_info().BoardID,"{\"cmd\":\"OFF\"}");
                                }
                           // }    
                    }  
                    _asyncMqttClientService.publish(_topicCommandResponse,"{\""+platformio_api::get_device_info().BoardID+"\":\"OK\"}");         
                }else if(_topicSplit.size()== 3){
                     _loRaService.sendMessage(_topicSplit[2],platformio_api::get_device_info().BoardID,"{\"cmd\":\""+product_api::get_ips_protocol().mode+"\"}");
                     TracePrinter::printTrace(String("[GW]:MQTT:[send_timeout]:"+mail->payload)); 
                }else{
                    TracePrinter::printTrace(String("[GW]:MQTT:[x]:"+mail->topic));
                }
            }else{
                TracePrinter::printTrace(String("[GW]:MQTT:[x]:JsonParse ERROR..."));
            }
            _mail_box_mqtt.free(mail); 
        }
    }
}


void LoRaGateway::onMqttConnectCallback(bool sessionPresent)
{
    TracePrinter::printTrace("[GW]MQTT::onMqttConnectCallback");

}
void LoRaGateway::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    TracePrinter::printTrace("[GW]MQTT:onMqttDisconnectCallback");
}
void LoRaGateway::onMqttMessageCallback(const String& topic,const String& payload)
{
    mqtt::mail_on_message_t *mail =  _mail_box_mqtt.alloc();
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
            TracePrinter::printTrace("[GW]lora:Rx:"+mail->receiver+String(":Tx:")+mail->sender);
            if(mail->receiver == platformio_api::get_device_info().BoardID){
                 DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                TracePrinter::printTrace(mail->packet);
                if (!error)
                { 
                    TracePrinter::printTrace("[#]lora GW: JsonParse OK!");
                }else{
                    TracePrinter::printTrace("[x]lora GW: JsonParse ERROR...");
                }
                
            }else if(mail->receiver == String("FAFA")){
                TracePrinter::printTrace("[GW]lora:Rx:FAFA:n/a");
            }
            else{
                TracePrinter::printTrace("[GW]lora:Rx:[x]:n/a");
            }
            
            
            _mail_box_lora.free(mail); 
        }
    }
}
void LoRaGateway::onLoRaMessageCallback(const lora::mail_t& lora_mail)
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





