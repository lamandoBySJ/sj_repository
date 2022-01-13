#include "SmartBox.h"
#include "FS.h"
#include "FFat.h"
#include "FFatHelper.h"
#include "app/RTC/RTC.h"
#include "app/RTC/RTCBase.h"
#include "app/RTC/TimeDirector.h"
#include "app/RTC/DS1307Builder.h" 

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

    platformio_api::get_web_properties().ap_ssid = Device::WiFiMacAddress();
    PlatformDebug::println("DeviceInfo::BoardID:"+Device::WiFiMacAddress());

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
  if(FatHelper.readFile(FFat,User::getProperties().path,text)){
        PlatformDebug::println(text); 
        DeserializationError error = deserializeJson(docProperties, text);
        if(!error){
            User::getProperties().ssid =  docProperties["ssid"].as<String>();
            User::getProperties().pass =  docProperties["pass"].as<String>();
            User::getProperties().host =  docProperties["host"].as<String>();
            User::getProperties().port =  docProperties["port"].as<int>();
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

    PlatformDebug::println("user_properties::path:"+User::getProperties().path);
    PlatformDebug::println("user_properties::ssid:"+User::getProperties().ssid);
    PlatformDebug::println("user_properties::pass:"+User::getProperties().pass);
    PlatformDebug::println("user_properties::host:"+User::getProperties().host);
    PlatformDebug::println(String(User::getProperties().port,DEC));

    guard::LoopTaskGuard::getLoopTaskGuard().init();

    LED::io_state_reset();
    bool ok = RTC::BuildSchedule(DS1307Builder (_mtx,Wire,(uint8_t)13) );

    
    ok ? Logger::cleanup_errors() : Logger::error(__PRETTY_FUNCTION__,__LINE__);
    LED::io_state(LedName::RTC,ok);

    _colorSensor.power_on();
    Wire1.begin(4,15,100000);
    ok = _colorSensor.init();
    LED::io_state(LedName::ALS,ok);

    time_t t= RTC::LocalDateTime::now(text);
    TracePrinter::printf("\n---------------- %d,%s----------------\n",t,text.c_str());

    TracePrinter::printTrace("\n---------------- "+platformio_api::get_version()+" ----------------\n");
}





void SmartBox::color_measure(MeasEventType type)
{   
    colorCollector.post_mail_measure(type,nullptr);
}

void  SmartBox::startup(){
    
    try{
        colorCollector.startup();

      _topics.insert(Device::WiFiMacAddress()+"/ServerTime");
      _topics.insert(Device::WiFiMacAddress()+"/ServerReq");

      WiFiService::startup(User::getProperties().ssid.c_str(),User::getProperties().pass.c_str(),this);

      AsyncMqttClientService::startup(User::getProperties().host.c_str(),User::getProperties().port,this);

    }catch(const os::thread_error& e){
        TracePrinter::printTrace(e.what());
        Logger::error(__FUNCTION__,__LINE__);
    }
    
}

void  SmartBox::start_web_service()
{   
  
  if(!espWebService.isRunning()){
    std::lock_guard<rtos::Mutex> lck(_mtxNetwork);

    LED::io_state(LedName::WEB,true);
    
    guard::LoopTaskGuard::getLoopTaskGuard().loop_stop();

    if(AsyncMqttClientService::connected()){
        AsyncMqttClientService::disconnect();
    }

    WiFiService::setSoftAP(platformio_api::get_web_properties().ap_ssid.c_str(),platformio_api::get_web_properties().ap_pass.c_str());
    
    WiFiService::switchWifiMode(WIFI_MODE_AP);

    ThisThread::sleep_for(1000);

    TimeoutManager::remove(this);
    
    colorCollector.setCallbackWebSocketClientText(mbed::callback(&espWebService,&ESPWebService::delegateMethodWebSocketClientText));
    colorCollector.setCallbackWebSocketClientEvent(mbed::callback(&espWebService,&ESPWebService::delegateMethodWebSocketClientEvent));
    
    espWebService.addCallbackOnWsEvent(mbed::callback(&colorCollector,&ColorCollector::delegateMethodOnWsEvent));
    espWebService.startup();  

    espWebService.set_start_signal();
   
  }
}
void SmartBox::start_http_update(const String& url){
      
      guard::LoopTaskGuard::getLoopTaskGuard().loop_stop();

 
      ESPhttpUpdate.rebootOnUpdate(false);
      t_httpUpdate_return ret =  ESPhttpUpdate.update(url);
      
      switch(ret) {
        case HTTP_UPDATE_FAILED:
            PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
           // AsyncMqttClientService::publish("upgrade/status/"+Device::WiFiMacAddress(),"{\"status\":\"ESP_OTA_FAIL\"}");
            break;
         case HTTP_UPDATE_NO_UPDATES:
            PlatformDebug::println("HTTP_UPDATE_NO_UPDATES");
           // AsyncMqttClientService::publish("upgrade/status/"+Device::WiFiMacAddress(),"{\"status\":\"ESP_OTA_FAIL\"}");
            break;
        case HTTP_UPDATE_OK:
            PlatformDebug::println("HTTP_UPDATE_OK");
           // AsyncMqttClientService::publish("upgrade/status/"+Device::WiFiMacAddress(),"{\"status\":\"ESP_OTA_OK\"}");
            break;
        default:
            PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
      }

      ESP.restart();
}
void SmartBox::start_https_update(const String& url){
      ESPhttpUpdate.rebootOnUpdate(false);
      t_httpUpdate_return ret =  ESPhttpUpdate.update(url);
      switch(ret) {
        case HTTP_UPDATE_FAILED:
            PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            AsyncMqttClientService::publish("upgrade/status/"+Device::WiFiMacAddress(),"{\"status\":\"ESP_OTA_FAIL\"}");
            break;
         case HTTP_UPDATE_NO_UPDATES:
            PlatformDebug::println("HTTP_UPDATE_NO_UPDATES");
            AsyncMqttClientService::publish("upgrade/status/"+Device::WiFiMacAddress(),"{\"status\":\"ESP_OTA_FAIL\"}");
            break;
        case HTTP_UPDATE_OK:
            PlatformDebug::println("HTTP_UPDATE_OK");
            AsyncMqttClientService::publish("upgrade/status/"+Device::WiFiMacAddress(),"{\"status\":\"ESP_OTA_OK\"}");
            break;
        default:break;
      }
      ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
      ESP.restart();
}


