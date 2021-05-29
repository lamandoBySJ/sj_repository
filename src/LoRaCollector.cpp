#include "LoRaCollector.h"

void LoRaCollector::startup(WirelessTechnologyType type)
{
    _topicCommandResponse = platformio_api::get_device_info().Family+ String("/command/response/DC");
    _topicCommandRequest = platformio_api::get_device_info().Family+ String("/command/request/DC");
   _topicSendRssi = platformio_api::get_device_info().Family+String("/send_rssi/#");
  
    _topics.push_back(_topicSendRssi);
    _topics.push_back(_topicCommandRequest);

    if(type == WirelessTechnologyType::WiFi){
        _threadMqttService.start(callback(this,&LoRaCollector::run_mqtt_service));
      // _thread = std::thread(&LoRaCollector::run_mqtt_service,this);
    }else{
       _threadLoraService.start(callback(this,&LoRaCollector::run_lora_service));
    }
  
   _threadBackgroundService.start(callback(this,&LoRaCollector::run_background_service));
}
void LoRaCollector::run_mqtt_service()
{
    std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
    while(true){
        osEvent evt= _mail_box_mqtt.get();
        if (evt.status == osEventMail) {

            lck.lock();

            mqtt::mail_on_message_t *mail = (mqtt::mail_on_message_t *)evt.value.p;
            TracePrinter::printTrace("[DC]MQTT:"+mail->topic);
            TracePrinter::printTrace("[DC]MQTT:"+mail->payload);
            
           
            _topicSplit.clear();
            StringHelper::split( _topicSplit,mail->topic.c_str(),"/");
            DynamicJsonDocument  doc(mail->payload.length()+1024);
            DeserializationError error = deserializeJson(doc,mail->payload);
          
           
           if(_topicSplit.size() == 3){
                if (_topicSplit[1] == "send_rssi") {
                    if (!error)
                    {   
                        String tagID;
                        String beaconID =  _topicSplit[2];
                        for(JsonPair v: doc.as<JsonObject>()){
                            tagID = v.key().c_str();
                            _mapOnlineDevices[tagID][beaconID]=v.value().as<int>();
                            break;
                        }
                        _mapDataCollector[tagID].insert(beaconID);
                        _mapTagTimeout[tagID]=millis();
                        if(_mapSetupBeacons.size() == _mapDataCollector[tagID].size()){
                            background::mail_t* sig = _mail_box_background_signal.alloc();
                            sig->TAG_ID = tagID;
                            _mail_box_background_signal.put(sig);
                        }
                        
                    }   
                }
            
            }else if(_topicSplit.size() == 4){
            
                        if(doc.containsKey("beacons")){
                            _mapSetupBeacons.clear();
                            for(auto v :doc["beacons"].as<JsonArray>()){
                                for(JsonPair p : v.as<JsonObject>()){
                                    _mapSetupBeacons[p.key().c_str()]=p.value().as<String>();
                                    TracePrinter::printTrace("[DC]MQTT:beaconID:"+String(p.key().c_str()));
                                }
                            }
                            if(FatHelper.writeFile(FFat,product_api::get_beacon_properties().path,doc.as<String>())){
                                TracePrinter::printTrace("[DC]MQTT:W:beacons:OK");
                            }else{
                                TracePrinter::printTrace("[DC]MQTT:W:beacons:ERROR");
                            }
                        }else  if(doc.containsKey("tags")){
                            _mapTagLocation.clear();
                            for(auto v :doc["tags"].as<JsonArray>()){
                                for(JsonPair p : v.as<JsonObject>()){
                                    _mapTagLocation[p.key().c_str()]=p.value().as<String>();
                                    TracePrinter::printTrace("[DC]MQTT:tagID:"+String(p.key().c_str())+",Location:"+p.value().as<String>());
                                }
                            }
                         
                        }else if(doc.containsKey("technology")){
                            product_api::get_ips_protocol().technology = doc["technology"].as<String>();
                            product_api::get_ips_protocol().family     = doc["family"].as<String>();
                            product_api::get_ips_protocol().gateway    = doc["gateway"].as<String>();
                            product_api::get_ips_protocol().collector  = doc["collector"].as<String>();
                            product_api::get_ips_protocol().mode       = doc["mode"].as<String>();
                        }
              
                _networkService.publish(_topicCommandResponse,"{\""+platformio_api::get_device_info().BoardID+"\":\"OK\"}");
            }
            
           
            _mail_box_mqtt.free(mail); 
            lck.unlock();
        }
    }
}

void LoRaCollector::run_background_service()
{   
   
 
    while(true)
    {
       
        osEvent evt = _mail_box_background_signal.get(2000);

        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();

        if (evt.status == osEventMail) {
             TracePrinter::printTrace("[DC]MQTT:BKGD:osEventMail:"+String(millis()-_millis,DEC));
            // _mail_box_background_signal.free((int*)evt.value.p);
            background::mail_t *mail_background = (background::mail_t *)evt.value.p;
            _payload="{";
            for(auto& v: _mapOnlineDevices[mail_background->TAG_ID]){
                _payload+=v.first;
                _payload+=":";
                _payload+=v.second;
                _payload+=",";
            }
            _payload+="}";
            if( product_api::get_ips_protocol().mode=="learn"){
                _topicLT =  product_api::get_ips_protocol().family+String("/")+ product_api::get_ips_protocol().mode+String("/")+mail_background->TAG_ID+String("/")+_mapTagLocation[mail_background->TAG_ID];
            }else{
                _topicLT =  product_api::get_ips_protocol().family+String("/track/")+mail_background->TAG_ID;
            }
                  
            _networkService.publish(_topicLT,_payload);

            _mapDataCollector[mail_background->TAG_ID].clear();
            _mapDataCollector.erase(mail_background->TAG_ID);

            _mapOnlineDevices[mail_background->TAG_ID].clear();
            _mapOnlineDevices.erase(mail_background->TAG_ID);
                 
            if(_mapTagTimeout.find(mail_background->TAG_ID)!=_mapTagTimeout.end()){
                _mapTagTimeout.erase(mail_background->TAG_ID);
            }
            
            //_mapTagTimeout.clear();
            _mail_box_background_signal.free(mail_background); 

        }else if (evt.status == osEventTimeout){
 
            for(auto& tagTimeout:_mapTagTimeout){
                for(auto& v : _mapSetupBeacons){
                        TracePrinter::printTrace("v:"+v.first);
                }
                if(millis()-tagTimeout.second > 2000){
                    
                    DynamicJsonDocument  d(_mapOnlineDevices.size()*10+1024);
                    JsonArray array =  d.createNestedArray(tagTimeout.first);

                    for(auto& v: _mapOnlineDevices[tagTimeout.first]){
                        JsonObject obj=array.createNestedObject();
                        obj[v.first]=v.second;
                    }
                    String _fingerprints=d.as<String>(); 
     
                    if(_mapSetupBeacons.size() == _mapDataCollector[tagTimeout.first].size()){
                            _payload="{";
                            for(auto& v: _mapOnlineDevices[tagTimeout.first]){
                                _payload+=v.first;
                                _payload+=":";
                                _payload+=v.second;
                                _payload+=",";
                            }
                            _payload+="}";
                            if( product_api::get_ips_protocol().mode=="learn"){
                                _topicLT =  product_api::get_ips_protocol().family+String("/")+ product_api::get_ips_protocol().mode+String("/")+tagTimeout.first+String("/")+_mapTagLocation[tagTimeout.first];
                            }else{
                                _topicLT =  product_api::get_ips_protocol().family+String("/track/")+tagTimeout.first;
                            }
                                
                            _networkService.publish(_topicLT,_payload);
                            
                    }else{
                            _topicTimeout = product_api::get_ips_protocol().family+String("/send_timeout/"+tagTimeout.first);
                            _networkService.publish(_topicTimeout,_fingerprints);
                    }
                    
                    _mapDataCollector[tagTimeout.first].clear();
                    _mapDataCollector.erase(tagTimeout.first);

                    _mapOnlineDevices[tagTimeout.first].clear();
                    _mapOnlineDevices.erase(tagTimeout.first);
                                
                    if(_mapTagTimeout.find(tagTimeout.first)!=_mapTagTimeout.end()){
                        _mapTagTimeout.erase(tagTimeout.first);
                    }
                   // _mapTagTimeout.clear();
                }
            }     
        }
        lck.unlock();	
    }      
}

void LoRaCollector::onMqttConnectCallback(bool sessionPresent)
{
    TracePrinter::printTrace("[DC]]MQTT:onMqttConnectCallback");
}
void LoRaCollector::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    TracePrinter::printTrace("[DC]]MQTT:onMqttDisconnectCallback");
}
void LoRaCollector::onMessageMqttCallback(const String& topic,const String& payload)
{   

    mqtt::mail_on_message_t *mail =  _mail_box_mqtt.alloc();
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
            TracePrinter::printTrace("[DC]lora:Rx:"+mail->receiver+String(":Tx:")+mail->sender);
            if(mail->receiver == platformio_api::get_device_info().BoardID){
                
                DynamicJsonDocument  doc(mail->packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->packet);
                if (!error)
                { 
                    TracePrinter::printTrace(mail->sender+String(":")+String(mail->rssi,DEC));
                }else{
                    TracePrinter::printTrace("[DC]lora:[x]:JsonParse ERROR...");
                }
            }else if(mail->receiver == String("FAFA")){
                TracePrinter::printTrace("[DC]lora:FAFA:n/a");
            }else{
                TracePrinter::printTrace("[DC]lora:n/a");
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







