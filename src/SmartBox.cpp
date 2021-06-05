#include "SmartBox.h"
#include "FS.h"
#include "FFat.h"
#include "FFatHelper.h"
//extern String& platformio::api::version(const char* date,const char* time);

extern FFatHelper<rtos::Mutex> FatHelper;

void SmartBox::platformio_init()
{   
  
    #if CONFIG_SUPPORT_STATIC_ALLOCATION
    throw os::alloc_error(String("[x]:os do not support static allocation.\nerror in file: ")+String("file:")+String(__FILE__)+String(",line:")+String(__LINE__,DEC));
    #endif
  
    esp_sleep_wakeup_cause_t cause =  esp_sleep_get_wakeup_cause();
    switch(cause){
      case ESP_SLEEP_WAKEUP_UNDEFINED:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_UNDEFINED");
      break;    //!< In case of deep sleep, reset was not caused by exit from deep sleep
      case ESP_SLEEP_WAKEUP_ALL:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_ALL");
      break;           //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
      case ESP_SLEEP_WAKEUP_EXT0:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_EXT0");
      break;          //!< Wakeup caused by external signal using RTC_IO
      case ESP_SLEEP_WAKEUP_EXT1:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_EXT1");
      break;         //!< Wakeup caused by external signal using RTC_CNTL
      case ESP_SLEEP_WAKEUP_TIMER:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_TIMER");
      break;         //!< Wakeup caused by timer
      case ESP_SLEEP_WAKEUP_TOUCHPAD:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_TOUCHPAD");
      break;      //!< Wakeup caused by touchpad
      case ESP_SLEEP_WAKEUP_ULP:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_ULP");
      break;           //!< Wakeup caused by ULP program
      case ESP_SLEEP_WAKEUP_GPIO:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_GPIO");
      break;         //!< Wakeup caused by GPIO (light sleep only)
      case ESP_SLEEP_WAKEUP_UART:
      PlatformDebug::println("ESP_SLEEP_WAKEUP_UART");
      break; 
      default:break;
    }

    String mac_address=WiFi.macAddress();
    mac_address.replace(":","");
    platformio_api::get_web_properties().ap_ssid = WiFi.macAddress();
    PlatformDebug::println("DeviceInfo::BoardID:"+mac_address);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    platformio_api::get_device_info().BoardID = mac_address;
    platformio_api::get_device_info().Family = "k49a";
    PlatformDebug::println(platformio_api::get_device_info().BoardID);

    if(FatHelper.init()){
        PlatformDebug::println("OK:File system mounted");
    }else{
        PlatformDebug::println("ERROR:File system:");
    }

    //FatHelper.deleteFiles(FFat, "/", 0);
    FatHelper.listDir(FFat, "/", 0);
    if(!FatHelper.exists("/TowerColor")){
        FatHelper.createDir(FFat,"/TowerColor");
        PlatformDebug::println("Dir created:/TowerColor");
    }
    if(!FatHelper.exists("/data")){
        FatHelper.createDir(FFat,"/data");
        PlatformDebug::println("Dir created:/data");
    }
    

    DynamicJsonDocument  docProperties(1024);
    String text;
  if(FatHelper.readFile(FFat,platformio_api::get_user_properties().path,text)){
        PlatformDebug::println(text); 
        DeserializationError error = deserializeJson(docProperties, text);
        if(!error){
            platformio_api::get_user_properties().ssid =  docProperties["ssid"].as<String>();
            platformio_api::get_user_properties().pass =  docProperties["pass"].as<String>();
            platformio_api::get_user_properties().host =  docProperties["host"].as<String>();
            platformio_api::get_user_properties().port =  docProperties["port"].as<int>();
        }
    }

    if(FatHelper.readFile(FFat,product_api::get_rgb_properties().path,text)){
        DeserializationError error= deserializeJson(docProperties, text);
        if(!error){
            product_api::get_rgb_properties().r_offset =  docProperties["r_offset"].as<uint16_t>();
            product_api::get_rgb_properties().g_offset =  docProperties["g_offset"].as<uint16_t>();
            product_api::get_rgb_properties().b_offset =  docProperties["b_offset"].as<uint16_t>();
            PlatformDebug::println("rgb_properties::r_offset:"+String(product_api::get_rgb_properties().r_offset));
            PlatformDebug::println("rgb_properties::g_offset:"+String(product_api::get_rgb_properties().g_offset));
            PlatformDebug::println("rgb_properties::b_offset:"+String(product_api::get_rgb_properties().b_offset));
        }
    }

    PlatformDebug::println("user_properties::path:"+platformio_api::get_user_properties().path);
    PlatformDebug::println("user_properties::ssid:"+platformio_api::get_user_properties().ssid);
    PlatformDebug::println("user_properties::pass:"+platformio_api::get_user_properties().pass);
    PlatformDebug::println("user_properties::host:"+platformio_api::get_user_properties().host);
    PlatformDebug::println(String(platformio_api::get_user_properties().port,DEC));

    TracePrinter::init();

    guard::LoopTaskGuard::getLoopTaskGuard();

    LEDIndicator::getLEDIndicator().io_state_reset();

    attachInterrupt(0,&guard::LoopTaskGuard::set_signal_web_service,FALLING);

    Wire.begin(21,22);
    Wire1.begin(4,15);
    bool ok;
    ok = _rtc.init(true,CLOCK_H24); 
    LEDIndicator::getLEDIndicator().io_state_rtc(ok);

    ok = _colorSensor.init(true);
    LEDIndicator::getLEDIndicator().io_state_als(ok);
    
    TracePrinter::printTrace("\n---------------- "+String(__DATE__)+" "+String(__TIME__)+" ----------------\n");
}



void  SmartBox::task_collection_service(){
      TracePrinter::printTrace("(*collection_->startup()");
      while(true){
          ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));
      }
  }
void SmartBox::color_measure()
{   
    colorCollector.post_mail_measure(MeasEventType::EventSystemMeasure,nullptr);
}



void  SmartBox::startup(){
    
    try{
      // _countdownTimeSync.startup();

      
       _topics.insert(platformio_api::get_device_info().BoardID+"/ServerTime");
       _topics.insert(platformio_api::get_device_info().BoardID+"/ServerReq");

       wifiService.addOnWiFiServiceCallback(mbed::callback(this,&SmartBox::onWiFiServiceCallback));
       networkService.addOnMqttDisonnectCallback(mbed::callback(this,&SmartBox::onMqttDisconnectCallback));
       networkService.addOnMqttConnectCallback(mbed::callback(this,&SmartBox::onMqttConnectCallback));
       networkService.addOnMqttMessageCallback(mbed::callback(this,&SmartBox::onMqttMessageCallback));
       networkService.addOnMqttSubscribeCallback(mbed::callback(this,&SmartBox::onMqttSubscribeCallback));
       networkService.init();
       networkService.startup();

       colorCollector.setCallbackMqttPublish(mbed::callback(&networkService,&NetworkService::delegateMethodPublish));
       colorCollector.startup();

       wifiService.startup();
       
      _threadCore.start(mbed::callback(this,&SmartBox::start_core_task));

    }catch(const os::thread_error& e){
        TracePrinter::printTrace(e.what());
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
  while(true)
  {
    osEvent evt=  _mail_box_on_mqtt_message.get();
    if (evt.status == osEventMail) {
        mqtt::mail_on_message_t* mail= (mqtt::mail_on_message_t*)evt.value.p;
        DeserializationError error = deserializeJson(doc, mail->payload); 
        if (error){
            _mail_box_on_mqtt_message.free(mail);
            continue;
        }
        StringHelper::split(_splitTopics,mail->topic.c_str(),"/");
        if(_splitTopics.size()!=2){
            _mail_box_on_mqtt_message.free(mail);
            continue;
        }

        if(_splitTopics[1]=="ServerTime"){
              if (doc.containsKey("unix_timestamp")) {
                  time_t ts =   doc["unix_timestamp"].as<uint32_t>();
                  if (ts > 28800) {
                      _rtc.setEpoch(ts);
                      guard::LoopTaskGuard::getLoopTaskGuard().loop_start();
                    //  _countdownTimeSync.shutdown();
                  } 
              }
        }else if(_splitTopics[1]=="ServerReq"){
              if (doc.containsKey("url")) {
                  start_http_update(doc["url"].as<String>());
              }else if( doc.containsKey("event_type") ){
                  event_type=doc["event_type"].as<String>();
                              switch( str_map_type[event_type] ){
                                case RequestType::SYSTEM_MEASURE:
                                {
                                  colorCollector.post_mail_measure(MeasEventType::EventSystemMeasure,nullptr);
                                  break;
                                }
                                 case RequestType::SERVER_MEASURE:
                                {
                                  colorCollector.post_mail_measure(MeasEventType::EventServerMeasure,nullptr);
                                  break;
                                }
                                case RequestType::MANUAL_REQUEST:
                                {
                                  colorCollector.post_mail_measure(MeasEventType::EventManulRequest,nullptr);
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
        _mail_box_on_mqtt_message.free(mail);  
    }
  }
}

void SmartBox::onMqttMessageCallback(const String& topic,const String& payload)
{   
      mqtt::mail_on_message_t* mail=  _mail_box_on_mqtt_message.alloc();
      mail->topic = topic;
      mail->payload = payload;
      _mail_box_on_mqtt_message.put(mail);
}
void SmartBox::onMqttConnectCallback(bool sessionPresent)
{
    
    TracePrinter::printTrace("SmartBox::OK: Connected to MQTT.");
    LEDIndicator::getLEDIndicator().io_state_mqtt(true);
    for(auto topic:_topics){
      TracePrinter::printTrace("subscribe topic:"+topic);
      networkService.subscribe(topic);
    }
    //_countdown.shutdown();
}
void SmartBox::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{
    LEDIndicator::getLEDIndicator().io_state_mqtt(false);
    guard::LoopTaskGuard::getLoopTaskGuard().loop_stop();
  //  _countdown.startup();
    wifiService.switch_wifi_mode(WIFI_STA,SYSTEM_EVENT_STA_DISCONNECTED);
}
void SmartBox::onMqttSubscribeCallback(uint16_t packetId, uint8_t qos)
{
    if(packetId == _topics.size()){
      String invoke_data=String("{\"DeviceID\":\"")+platformio_api::get_device_info().BoardID+
      String("\",\"version\":\"")+platformio_api::get_version()+String("\",\"wifi_channel\":")+
      String(wifiService.getWiFiChannel(),DEC)+String("}");
      networkService.publish("SmartBox/TimeSync",invoke_data);
    }
}


void  SmartBox::start_web_service()
{   
  std::lock_guard<rtos::Mutex> lck(_mtx);
  if(!espWebService.isRunning()){
    LEDIndicator::getLEDIndicator().io_state_web(true);
    networkService.cleanupCallbacks();
    guard::LoopTaskGuard::getLoopTaskGuard().loop_stop();
    

    colorCollector.setCallbackWebSocketClientText(mbed::callback(&espWebService,&ESPWebService::delegateMethodWebSocketClientText));
    colorCollector.setCallbackWebSocketClientEvent(mbed::callback(&espWebService,&ESPWebService::delegateMethodWebSocketClientEvent));
    espWebService.addCallbackOnWsEvent(mbed::callback(&colorCollector,&ColorCollector::delegateMethodOnWsEvent));
    espWebService.startup();  
    wifiService.switch_wifi_mode(WIFI_AP_STA);
  }
}
void SmartBox::start_http_update(const String& url){
      std::lock_guard<rtos::Mutex> lck(_mtx);
      guard::LoopTaskGuard::getLoopTaskGuard().loop_stop();

      networkService.publish("upgrade/status/"+platformio_api::get_device_info().BoardID,"{\"status\":\"ESP_OTA_BEGIN\"}");
      ThisThread::sleep_for(Kernel::Clock::duration_seconds(2));
      networkService.shutdown();
 

      //Countdown timeoutChecker("http_update",90);
     // timeoutChecker.startup();
      ESPhttpUpdate.rebootOnUpdate(false);
      t_httpUpdate_return ret =  ESPhttpUpdate.update(url);
      PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      networkService.connect();
      do{
        ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
        
      }while(!networkService.connected());

      switch(ret) {
        case HTTP_UPDATE_FAILED:
            PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
             networkService.publish("upgrade/status/"+platformio_api::get_device_info().BoardID,"{\"status\":\"ESP_OTA_FAIL\"}");
            break;
         case HTTP_UPDATE_NO_UPDATES:
            PlatformDebug::println("HTTP_UPDATE_NO_UPDATES");
            networkService.publish("upgrade/status/"+platformio_api::get_device_info().BoardID,"{\"status\":\"ESP_OTA_FAIL\"}");
            break;
        case HTTP_UPDATE_OK:
            PlatformDebug::println("HTTP_UPDATE_OK");
            networkService.publish("upgrade/status/"+platformio_api::get_device_info().BoardID,"{\"status\":\"ESP_OTA_OK\"}");
            break;
        default:break;
      }
      ThisThread::sleep_for(Kernel::Clock::duration_seconds(2));
      ESP.restart();
}
void SmartBox::start_https_update(const String& url){
      ESPhttpUpdate.rebootOnUpdate(false);
      t_httpUpdate_return ret =  ESPhttpUpdate.update(url);
      switch(ret) {
        case HTTP_UPDATE_FAILED:
            PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            networkService.publish("upgrade/status/"+platformio_api::get_device_info().BoardID,"{\"status\":\"ESP_OTA_FAIL\"}");
            break;
         case HTTP_UPDATE_NO_UPDATES:
            PlatformDebug::println("HTTP_UPDATE_NO_UPDATES");
            networkService.publish("upgrade/status/"+platformio_api::get_device_info().BoardID,"{\"status\":\"ESP_OTA_FAIL\"}");
            break;
        case HTTP_UPDATE_OK:
            PlatformDebug::println("HTTP_UPDATE_OK");
            networkService.publish("upgrade/status/"+platformio_api::get_device_info().BoardID,"{\"status\":\"ESP_OTA_OK\"}");
            break;
        default:break;
      }
      ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
      ESP.restart();
  }


