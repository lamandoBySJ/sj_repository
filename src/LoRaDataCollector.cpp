#include "LoRaDataCollector.h"

void LoRaDataCollector::startup()
{
   _topicSendRssi = platform_debug::DeviceInfo::Family+String("/send_rssi");
   _topicLT = platform_debug::DeviceInfo::Family+String("/send_fingerprint");
    _topics.push_back(_topicSendRssi);
    _threadMqttService.start(callback(this,&LoRaDataCollector::run_mqtt_service));
   // _threadLoraService.start(callback(this,&LoRaDataCollector::run_lora_service));
   _threadBackgroundService.start(callback(this,&LoRaDataCollector::run_background_service));
}
void LoRaDataCollector::run_mqtt_service()
{
    
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;
            
            String tagID;
            String beaconID;
                    
            bool timeout=false;
            if(mail->counter == 255){
                timeout=true;
                tagID = mail->topic;
                platform_debug::TracePrinter::printTrace(String("LoRa DC:counter Timeout:0xFF Timeout:"+tagID));
            }else if (mail->topic == _topicSendRssi) {
                DynamicJsonDocument  doc(mail->payload.length()+128);
                DeserializationError error = deserializeJson(doc,mail->payload);
                if (!error)
                { 
                    tagID =  doc["tagID"].as<String>();
                    beaconID =  doc["beaconID"].as<String>();
                    _mapTrackDevices[tagID][beaconID]=doc["rssi"].as<int>();
                    _mapTagCounter[tagID].insert(beaconID);

                    _mutex.lock();
                    if(_setBeaconCollector.find(tagID) ==_setBeaconCollector.end()){
                         _setBeaconCollector.insert(tagID);
                         background::mail_t *mail_background =  _mail_box_background.alloc();
                        if(mail_background!=NULL){
                            mail_background->TAG_ID = tagID;
                             _mail_box_background.put(mail_background) ;
                         }
                    }
                    _mutex.unlock();
                }   
            }

            if(String("")!=tagID ){
                    if(_mapSetupBeacons.size() ==_mapTagCounter[tagID].size() || timeout){

                         _mutex.lock();
                         _setBeaconCollector.erase(tagID);
                        _mutex.unlock();
                        DynamicJsonDocument  doc(_mapTrackDevices.size()*10+128);
                        JsonArray array =  doc.createNestedArray(tagID);
                        for(auto& v: _mapTrackDevices[tagID]){
                            JsonObject obj=array.createNestedObject();
                            obj[v.first]=v.second;
                        }
                        _mapTagCounter[tagID].clear();
                        _mapTagCounter.erase(tagID);
                        _mapTrackDevices[tagID].clear();
                        _mapTrackDevices.erase(tagID);
                        _fingerprints=doc.as<String>();  
                        platform_debug::TracePrinter::printTrace( _fingerprints);
                        _mqttNetwork.publish(_topicLT,_fingerprints);

                    }
            }
            _mail_box_mqtt.free(mail); 
        }
    }
}

void LoRaDataCollector::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("LoRa Gateway Master Runnning...");
}
void LoRaDataCollector::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("LoRa Gateway Master Abort...");
}
void LoRaDataCollector::onMessageMqttCallback(const String& topic,const String& payload)
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


void LoRaDataCollector::run_lora_service()
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
                }else{
                    platform_debug::TracePrinter::printTrace("lora DC: JsonParse ERROR...");
                }
            }else if(mail->receiver == String("FAFA")){
               
                platform_debug::TracePrinter::printTrace("lora DC:FAFA:sendor:"+mail->sender);
            }else{
                platform_debug::TracePrinter::printTrace("[NA]lora DC:receiver:"+mail->receiver+String(",sender:")+mail->sender);
            }
            
            _mail_box_lora.free(mail); 
        }
    }
}
void LoRaDataCollector::onMessageLoRaCallback(const lora::mail_t& lora_mail)
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


void LoRaDataCollector::run_background_service()
{
    while(true){
        osEvent evt= _mail_box_background.get();
        if (evt.status == osEventMail) {
            background::mail_t *mail_background = (background::mail_t *)evt.value.p;
            ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));

             _mutex.lock();
            if(_setBeaconCollector.find(mail_background->TAG_ID) !=_setBeaconCollector.end()){
                mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
                if(mail!=NULL){
                    mail->counter = 0xFF;
                    mail->topic = mail_background->TAG_ID;
                    _mail_box_mqtt.put(mail) ;
                }
            }
            _mutex.unlock();
            _mail_box_background.free(mail_background); 
        }
    }
}




