#include "LoRaDataCollector.h"

void LoRaDataCollector::startup(WirelessTechnologyType type)
{
    _topicCommandResponse = DeviceInfo::Family+ String("/command/response/DC");
    _topicCommand = DeviceInfo::Family+ String("/command/request/DC");
   _topicSendRssi = platform_debug::DeviceInfo::Family+String("/send_rssi");
   _topicFP = platform_debug::DeviceInfo::Family+String("/send_fingerprint");
    _topics.push_back(_topicSendRssi);
    _topics.push_back(_topicCommand);

    
    if(type == WirelessTechnologyType::WiFi){
        _threadMqttService.start(callback(this,&LoRaDataCollector::run_mqtt_service));
    }else{
       _threadLoraService.start(callback(this,&LoRaDataCollector::run_lora_service));
    }
  
   _threadBackgroundService.start(callback(this,&LoRaDataCollector::run_background_service));
}
void LoRaDataCollector::run_mqtt_service()
{
    
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;
            platform_debug::TracePrinter::printTrace("[DC]MQTT:"+mail->topic);
            DynamicJsonDocument  doc(mail->payload.length()+1024);
            DeserializationError error = deserializeJson(doc,mail->payload);

            if (mail->topic == _topicSendRssi) {
                String tagID;
                String beaconID;
                if (!error)
                {   
                    tagID =  doc["tagID"].as<String>();
                    beaconID =  doc["beaconID"].as<String>();

                    _mutex.lock();
                    _mapTrackedDevices[tagID][beaconID]=doc["rssi"].as<int>();
                    _mapDataCollector[tagID].insert(beaconID);

                    if(_mapSetupBeacons.size() ==_mapDataCollector[tagID].size()){
                         _setBeaconCollector.erase(tagID);
                        DynamicJsonDocument  d(_mapTrackedDevices.size()*10+1024);
                        JsonArray array =  d.createNestedArray(tagID);
                        for(auto& v: _mapTrackedDevices[tagID]){
                            JsonObject obj=array.createNestedObject();
                            obj[v.first]=v.second;
                        }
                        _mapDataCollector[tagID].clear();
                        _mapDataCollector.erase(tagID);
                        _mapTrackedDevices[tagID].clear();
                        _mapTrackedDevices.erase(tagID);
                        _fingerprints=d.as<String>();  
                        platform_debug::TracePrinter::printTrace("[DC]MQTT:OK:"+ _fingerprints);

                        _mqttNetwork.publish(_topicFP,_fingerprints);

                    }else{
                       
                        if(_setBeaconCollector.find(tagID) == _setBeaconCollector.end()){
                            _setBeaconCollector.insert(tagID);
                            background::mail_t *mail_background =  _mail_box_background.alloc();
                            if(mail_background!=NULL){
                                mail_background->TAG_ID = tagID;
                                mail_background->currentMillis = millis();
                                _mail_box_background.put(mail_background) ;
                            }
                        }
                    }
                    _mutex.unlock();
                }   
            }else{
                if(doc.containsKey("beacons")){
                    _mapSetupBeacons.clear();
                    for(auto v :doc["beacons"].as<JsonArray>()){
                        for(JsonPair p : v.as<JsonObject>()){
                            _mapSetupBeacons[p.key().c_str()]=p.value().as<String>();
                            platform_debug::TracePrinter::printTrace("[DC]MQTT:beaconID:"+String(p.key().c_str()));
                        }
                    }
                    _mqttNetwork.publish(_topicCommandResponse,"{\""+DeviceInfo::BoardID+"\":\"OK\"}");    
                }
            }

            _mail_box_mqtt.free(mail); 
        }
    }
}

void LoRaDataCollector::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("[DC]]MQTT:onMqttConnectCallback");
}
void LoRaDataCollector::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("[DC]]MQTT:onMqttDisconnectCallback");
}
void LoRaDataCollector::onMessageMqttCallback(const String& topic,const String& payload)
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


void LoRaDataCollector::run_lora_service()
{
    while(true){
        osEvent evt= _mail_box_lora.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            platform_debug::TracePrinter::printTrace("[DC]lora:Rx:"+mail->receiver+String(":Tx:")+mail->sender);
            if(mail->receiver == platform_debug::DeviceInfo::BoardID){
                
                DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                if (!error)
                { 
                    platform_debug::TracePrinter::printTrace(mail->sender+String(":")+String(mail->rssi,DEC));
                }else{
                    platform_debug::TracePrinter::printTrace("[DC]lora:[x]:JsonParse ERROR...");
                }
            }else if(mail->receiver == String("FAFA")){
                platform_debug::TracePrinter::printTrace("[DC]lora:FAFA:n/a");
            }else{
                platform_debug::TracePrinter::printTrace("[DC]lora:n/a");
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
    long long diffTime = 0;
    while(true){
        osEvent evt= _mail_box_background.get();
        if (evt.status == osEventMail) {

                background::mail_t *mail_background = (background::mail_t *)evt.value.p;
           
                diffTime =  millis()-mail_background->currentMillis;
                if( diffTime < 0){
                    ThisThread::sleep_for(Kernel::Clock::duration_seconds(2000));
                }else if(diffTime < 2000){
                    ThisThread::sleep_for(Kernel::Clock::duration_seconds(2000-diffTime));
                }

                 _mutex.lock();

                if(_setBeaconCollector.find(mail_background->TAG_ID) !=_setBeaconCollector.end()){ 

                    DynamicJsonDocument  d(_mapTrackedDevices.size()*10+1024);
                    JsonArray array =  d.createNestedArray(mail_background->TAG_ID);
                    for(auto& v: _mapTrackedDevices[mail_background->TAG_ID]){
                        JsonObject obj=array.createNestedObject();
                        obj[v.first]=v.second;
                    }
                    int size = _mapTrackedDevices.size();
                    _mapDataCollector[mail_background->TAG_ID].clear();
                    _mapDataCollector.erase(mail_background->TAG_ID);
                    _mapTrackedDevices[mail_background->TAG_ID].clear();
                    _mapTrackedDevices.erase(mail_background->TAG_ID);
                    _fingerprints=d.as<String>();  
                    platform_debug::TracePrinter::printTrace("[x]BK:"+String(size,DEC)+":"+String(":")+_fingerprints);

                    _mqttNetwork.publish(_topicFP,_fingerprints);
                }else{
                    platform_debug::TracePrinter::printTrace("[~]BK:OK");
                }
                _mutex.unlock();
            
            _mail_box_background.free(mail_background); 
        }
    }
}




