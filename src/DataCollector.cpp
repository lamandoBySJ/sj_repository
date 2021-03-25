#include "DataCollector.h"

void DataCollector::startup()
{
    _threadMqttService.start(callback(this,&DataCollector::run_mqtt_service));
    _threadLoraService.start(callback(this,&DataCollector::run_lora_service));
}
void DataCollector::run_mqtt_service()
{
    
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;
            DynamicJsonDocument  doc(mail->payload.length()+128);
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                if (doc.containsKey("beacons")) {
                    _mapBeacons.clear();
                   JsonArray array = doc["beacons"].as<JsonArray>();
                   for(JsonObject obj : array){
                       for(JsonPair p : obj){
                            String key = p.key().c_str();
                            String val=  p.value().as<String>();
                            _mapBeacons[key]=std::map<String,int>();
                            platform_debug::TracePrinter::printTrace(key+String(":")+val);
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

void DataCollector::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("LoRa Gateway Master Runnning...");
}
void DataCollector::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("LoRa Gateway Master Abort...");
}
void DataCollector::onMessageMqttCallback(const String& topic,const String& payload)
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


void DataCollector::run_lora_service()
{
    while(true){
        osEvent evt= _mail_box_lora.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            if(mail->receiver == platform_debug::DeviceInfo::BoardID){
                platform_debug::TracePrinter::printTrace("lora DC:receiver:"+mail->receiver+String(",sender:")+mail->sender);
                DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                if (!error)
                { 
                    platform_debug::TracePrinter::printTrace(mail->sender+String(":")+String(mail->rssi,DEC));
                   
                    if (doc.containsKey("tag")) {
                        JsonObject obj =  doc["tag"].as<JsonObject>();
                        for (JsonPair p : obj) {
                             if(_mapBeacons.find(mail->sender) !=_mapBeacons.end()){
                                _mapBeacons[mail->sender][String(p.key().c_str())]=p.value().as<int>();

                                if(_mapRssiCounter.find(String(p.key().c_str())) !=_mapRssiCounter.end()){
                                    _mapRssiCounter[String(p.key().c_str())]++;

                                    if(_mapRssiCounter[String(p.key().c_str())] == _mapBeacons.size()){
                                        _fingerprints="";
                                        for(auto& v: _mapBeacons){
                                            _fingerprints+= v.first;
                                            _fingerprints+=String(":");
                                            _fingerprints+= String(v.second[String(p.key().c_str())],DEC);
                                            _fingerprints+=String(",");
                                        }
                                        _mapRssiCounter.erase(String(p.key().c_str()));
                                        platform_debug::TracePrinter::printTrace(_fingerprints);
                                    }
                                }
                             }
                        }
                        platform_debug::TracePrinter::printTrace(doc["tag"].as<String>());
                    } 
                }else{
                    platform_debug::TracePrinter::printTrace("lora DC: JsonParse ERROR...");
                }


            }else if(mail->receiver == String("FAFA")){
                platform_debug::TracePrinter::printTrace("lora DC:FAFA:sendor:"+mail->sender);
                if(_mapRssiCounter.find(mail->sender) == _mapRssiCounter.end()){
                    _mapRssiCounter[mail->sender]=0;
                }
            }else{
                platform_debug::TracePrinter::printTrace("[NA]lora DC:receiver:"+mail->receiver+String(",sender:")+mail->sender);
            }
            
            _mail_box_lora.free(mail); 
        }
    }
}
void DataCollector::onMessageLoRaCallback(const lora::mail_t& lora_mail)
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





