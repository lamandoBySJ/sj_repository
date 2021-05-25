#include "SmartBox.h"

extern FFatHelper<rtos::Mutex> FatHelper;
void  SmartBox::task_collection_service(){
      TracePrinter::printTrace("(*collection_->startup()");
      while(true){
          ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));
      }
  }

void  SmartBox::task_web_service()
{   
  if(!espWebService.isRunning()){
    colorCollector.setCallbackWebSocketClientText(callback(&espWebService,&ESPWebService::delegateMethodWebSocketClientText));
    colorCollector.setCallbackWebSocketClientEvent(callback(&espWebService,&ESPWebService::delegateMethodWebSocketClientEvent));
    espWebService.setCallbackPostMailToCollector(callback(&colorCollector,&ColorCollector::delegateMethodPostMail));
    espWebService.startup();  
  }
}

void  SmartBox::startup(){
    
    try{
       TimeMachine<DS1307,rtos::Mutex>::getTimeMachine()->init();  

       mqttNetwork.addOnMqttConnectCallback(callback(this,&SmartBox::onMqttConnect));
       mqttNetwork.addOnMessageCallback(callback(this,&SmartBox::onMessageMqttCallback));
       mqttNetwork.init();
       mqttNetwork.startup();

       colorCollector.startup();
     // _threadCore.start(callback(this,&SmartBox::start_core_task));

    }catch(const os::thread_error& e){
        TracePrinter::printTrace(e.what());
        //PlatformDebug::pause();
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(3000));  
    }

}
void SmartBox::start_core_task(){
    DynamicJsonDocument  doc(8192);
    TracePrinter::printTrace(String(ESP.getFreeHeap(),DEC));
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    //UBaseType_t x =uxTaskGetStackHighWaterMark(NULL);
    //TracePrinter::printTrace("stack left:"+String((int)x,DEC));
  String event_type;
  TracePrinter::printTrace("----------- core task -----------------");
  while(true)
  {
    osEvent evt=  _mail_box_mqtt.get();
    if (evt.status == osEventMail) {
        mqtt::mail_mqtt_t* mail= (mqtt::mail_mqtt_t*)evt.value.p;
        DeserializationError error = deserializeJson(doc, mail->payload); 
        if (error){
            _mail_box_mqtt.free(mail);
            continue;
        }
        if(_splitTopics[0]=="ServerTime"){
              if (!error && doc.containsKey("unix_timestamp")) {
                    uint32_t ts =   doc["unix_timestamp"];
                    if (ts > 28800) {
                       TimeMachine<DS1307,rtos::Mutex>::getTimeMachine()->setEpoch(ts);
                    }
              }
        }else if(_splitTopics[0]=="ServerReq"){
              if (doc.containsKey("url")) {
                  start_http_update(doc["url"].as<String>());
              }else if( doc.containsKey("event_type") ){
                  event_type=doc["event_type"].as<String>();
                              switch( str_map_type[event_type] ){
                                case RequestType::ALS_MEASURE:
                                {
                                  colorCollector.delegateMethodPostMail(MeasEventType::EventSystemMeasure,nullptr);
                                  break;
                                }
                                case RequestType::MANUAL_REQUEST:
                                {
                                 colorCollector.delegateMethodPostMail(MeasEventType::EventServerMeasure,nullptr);
                                  break;
                                }
                                case RequestType::OTA_CANCEL:
                                {
                                  break;
                                }
                                case RequestType::FILE_DOWNLOAD:
                                {
                                  if (doc.containsKey("file_url")) {
                                    String path = doc["file_url"].as<String>()+String("/");
                                    if (doc.containsKey("file_list")) {
                                        JsonArray filelist=doc["file_list"].as<JsonArray>();
                                        for(String filename :  filelist){
                                            _httpDownload.execute(path+filename,filename);
                                            ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(500));
                                        }                          
                                    }                  
                                  }
                                }break;
                                case RequestType::FILE_DELETE:
                                {
                                  if (doc.containsKey("file_list")) {
                                      JsonArray filelist=doc["file_list"].as<JsonArray>();
                                      for(String filename :  filelist){
                                        if(FatHelper.exists(filename) ){
                                          FatHelper.deleteFile(FFat,filename.c_str());
                                        }
                                      }
                                  }
                                  break;
                                }
                                default:break;
                              }                  
              }
        }
        _mail_box_mqtt.free(mail);  
    }
  }
}

void SmartBox::onMessageMqttCallback(const String& topic,const String& payload)
{   
    StringHelper::split(_splitTopics,topic.c_str(),"/");
    if(_splitTopics.size()==2){
      mqtt::mail_mqtt_t* mail=  _mail_box_mqtt.alloc();
      mail->topic = topic;
      mail->payload = payload;
      _mail_box_mqtt.put(mail);
    }
}
void SmartBox::onMqttConnect(bool sessionPresent)
{
    TracePrinter::printTrace("SmartBox::OK: Connected to MQTT.");
    for(auto topic:_topics){
      mqttNetwork.subscribe( topic);
    }
}

void SmartBox::start_http_update(const String& url){
      ESPhttpUpdate.rebootOnUpdate(false);
      t_httpUpdate_return ret =  ESPhttpUpdate.update(url);
      switch(ret) {
        case HTTP_UPDATE_FAILED:
            PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
         case HTTP_UPDATE_NO_UPDATES:
            PlatformDebug::println("HTTP_UPDATE_NO_UPDATES");
            break;
        case HTTP_UPDATE_OK:
            PlatformDebug::println("HTTP_UPDATE_OK");
            break;
        default:break;
      }
      ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
      ESP.restart();
}

void SmartBox::start_https_update(const String& url){
      ESPhttpUpdate.rebootOnUpdate(false);
      t_httpUpdate_return ret =  ESPhttpUpdate.update(url);
      switch(ret) {
        case HTTP_UPDATE_FAILED:
            PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
         case HTTP_UPDATE_NO_UPDATES:
            PlatformDebug::println("HTTP_UPDATE_NO_UPDATES");
            break;
        case HTTP_UPDATE_OK:
            PlatformDebug::println("HTTP_UPDATE_OK");
            break;
        default:break;
      }
      ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
      ESP.restart();
  }