#include "LoRaCollector.h"

void LoRaCollector::startup(WirelessTechnologyType type)
{
    _topicCommandResponse = DeviceInfo::Family+ String("/command/response/DC");
    _topicCommand = DeviceInfo::Family+ String("/command/request/DC");
   _topicSendRssi = platform_debug::DeviceInfo::Family+String("/send_rssi/#");
  
    _topics.push_back(_topicSendRssi);
    _topics.push_back(_topicCommand);

    
    if(type == WirelessTechnologyType::WiFi){
        _threadMqttService.start(callback(this,&LoRaCollector::run_mqtt_service));
    }else{
       _threadLoraService.start(callback(this,&LoRaCollector::run_lora_service));
    }
  
   _threadBackgroundService.start(callback(this,&LoRaCollector::run_background_service));
}
void LoRaCollector::run_mqtt_service()
{
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {
            mqtt::mail_t *mail = (mqtt::mail_t *)evt.value.p;
            platform_debug::TracePrinter::printTrace("[DC]MQTT:"+mail->topic);
            DynamicJsonDocument  doc(mail->payload.length()+1024);
            DeserializationError error = deserializeJson(doc,mail->payload);
            _topicSplit.clear();
            StringHelper::split( _topicSplit,mail->topic.c_str(),"/");
            if(_topicSplit.size() == 3){
                if (_topicSplit[1] == "send_rssi") {
                    if (!error)
                    {   
                        _mutex.lock();
                        String tagID;
                        String beaconID =  _topicSplit[2];
                        for(JsonPair v: doc.as<JsonObject>()){
                            tagID = v.key().c_str();
                            _mapOnlineDevices[tagID][beaconID]=v.value().as<int>();
                            break;
                        }

                        _mapDataCollector[tagID].insert(beaconID);
                        if(_mapDataCollector[tagID].size()==1){
                            _millis = millis();
                            background::mail_t *mail_background =  _mail_box_background.alloc();
                            if(mail_background!=NULL){
                                mail_background->counter = 1;
                                mail_background->TAG_ID = tagID;
                                _mail_box_background.put(mail_background) ;
                            }
                        }
 
                        if(_mapSetupBeacons.size() == _mapDataCollector[tagID].size()){

                            int* sig = _mail_box_signal.alloc();
                            _mail_box_signal.put(sig);
                        }
                        _mutex.unlock();
                    }   
                }
            
            }else if(_topicSplit.size() == 4){
               
                if(doc.containsKey("beacons")){
                    _mapSetupBeacons.clear();
                    for(auto v :doc["beacons"].as<JsonArray>()){
                        for(JsonPair p : v.as<JsonObject>()){
                            _mapSetupBeacons[p.key().c_str()]=p.value().as<String>();
                            platform_debug::TracePrinter::printTrace("[DC]MQTT:beaconID:"+String(p.key().c_str()));
                        }
                    }
                    
                }else  if(doc.containsKey("tags")){
                    _mapTagLocation.clear();
                    for(auto v :doc["tags"].as<JsonArray>()){
                        for(JsonPair p : v.as<JsonObject>()){
                            _mapTagLocation[p.key().c_str()]=p.value().as<String>();
                            platform_debug::TracePrinter::printTrace("[DC]MQTT:tagID:"+String(p.key().c_str())+",Location:"+p.value().as<String>());
                        }
                    }
                    
                }else if(doc.containsKey("technology")){
                    _IPSProtocol.technology = doc["technology"].as<String>();
                    _IPSProtocol.family     = doc["family"].as<String>();
                    _IPSProtocol.gateway    = doc["gateway"].as<String>();
                    _IPSProtocol.collector  = doc["collector"].as<String>();
                    _IPSProtocol.mode       = doc["mode"].as<String>();
                    
                }
                _mqttNetwork.publish(_topicCommandResponse,"{\""+DeviceInfo::BoardID+"\":\"OK\"}");
            }
            

            _mail_box_mqtt.free(mail); 
        }
    }
}

void LoRaCollector::run_background_service()
{   
    while(true){
        osEvent evt= _mail_box_background.get();
        if (evt.status == osEventMail) {

                background::mail_t *mail_background = (background::mail_t *)evt.value.p;
                osEvent event = _mail_box_signal.get(1000);
                 _mutex.lock();
                if (event.status == osEventMail) {
                    platform_debug::TracePrinter::printTrace("[DC]MQTT:BKGD:osEventMail:"+String(millis()-_millis,DEC));
                    _mail_box_signal.free((int*)evt.value.p);
                   
                    _payload="{";
                    for(auto& v: _mapOnlineDevices[mail_background->TAG_ID]){
                        _payload+=v.first;
                        _payload+=":";
                        _payload+=v.second;
                        _payload+=",";
                    }
                    _payload+="}";
                    if(_IPSProtocol.mode=="learn"){
                        _topicLT = _IPSProtocol.family+String("/")+_IPSProtocol.mode+String("/")+mail_background->TAG_ID+String("/")+_mapTagLocation[mail_background->TAG_ID];
                    }else{
                        _topicLT = _IPSProtocol.family+String("/track/")+mail_background->TAG_ID;
                    }
                  
                    _mqttNetwork.publish(_topicLT,_payload);

                    _mapDataCollector[mail_background->TAG_ID].clear();
                    _mapDataCollector.erase(mail_background->TAG_ID);

                    _mapOnlineDevices[mail_background->TAG_ID].clear();
                    _mapOnlineDevices.erase(mail_background->TAG_ID);
                    //_mqttNetwork.publish(_IPSProtocol.family+"/send_error/"+DeviceInfo::BoardID,_fingerprints);
                    if(_mapTimeoutExpired.find(mail_background->TAG_ID)!=_mapTimeoutExpired.end()){
                        _mapTimeoutExpired.erase(mail_background->TAG_ID);
                    }
                }else if (event.status == osEventTimeout){
                    
                    _mapDataCollector[mail_background->TAG_ID].clear();
                    _mapDataCollector.erase(mail_background->TAG_ID);

                    DynamicJsonDocument  d(_mapOnlineDevices.size()*10+1024);
                    JsonArray array =  d.createNestedArray(mail_background->TAG_ID);
                    for(auto& v: _mapOnlineDevices[mail_background->TAG_ID]){
                        JsonObject obj=array.createNestedObject();
                        obj[v.first]=v.second;
                    }
                    String _fingerprints=d.as<String>();  
              
                    platform_debug::TracePrinter::printTrace("[DC]:"+String(mail_background->counter,DEC)+":Timeout:0x"+String(event.status,HEX)+":"+String(":")+_fingerprints);
                    if( ++_mapTimeoutExpired[mail_background->TAG_ID] > 1){
                        _topicTimeoutExpired=_IPSProtocol.family+String("/send_countdown_expired/"+mail_background->TAG_ID);
                        _mqttNetwork.publish(_topicTimeoutExpired,_fingerprints);
                        _mapTimeoutExpired.erase(mail_background->TAG_ID);
                    }else{
                        _topicTimeout = _IPSProtocol.family+String("/send_timeout/"+mail_background->TAG_ID);
                        _mqttNetwork.publish(_topicTimeout,_fingerprints);
                    }
                }
                _mutex.unlock();
            _mail_box_background.free(mail_background); 
        }
    }
}

void LoRaCollector::onMqttConnectCallback(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("[DC]]MQTT:onMqttConnectCallback");
}
void LoRaCollector::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    platform_debug::TracePrinter::printTrace("[DC]]MQTT:onMqttDisconnectCallback");
}
void LoRaCollector::onMessageMqttCallback(const String& topic,const String& payload)
{   
    
    _topicMatch.clear();
     StringHelper::split(_topicMatch,topic.c_str(),"/");
    if(_topicMatch.size() == 3 && _topicMatch[1]!="send_rssi"){
        return;
    }else if(_topicMatch.size() == 4 && _topicMatch[3]!="DC"){
        return;
    }
    
    mqtt::mail_t *mail =  _mail_box_mqtt.alloc();
    if(mail!=NULL){
        mail->topic = topic;
        mail->payload = payload;
        _mail_box_mqtt.put(mail) ;
    }
}


void LoRaCollector::run_lora_service()
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
void LoRaCollector::onMessageLoRaCallback(const lora::mail_t& lora_mail)
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







