#include "IPSBox.h"
#include "Logger.h"
#include "Wire.h"
#include "FFatHelper.h"
#include "platform_debug.h"
#include "app/RTC/RTC.h"
#include "app/RTC/RTCBase.h"
#include "app/RTC/TimeDirector.h"
//#include "app/RTC/DS1307Builder.h" 
#include "app/RTC/DS3231MBuilder.h" 
#include "app/IOExpander/MCP.h"
#include "app/IOExpander/MCP23017Builder.h"
#include "app/LED/LED.h"
#include "LoopTaskGuard.h" 

void IPSBox::platformio_init()
{
    #if CONFIG_SUPPORT_STATIC_ALLOCATION
    throw os::alloc_error(String("[x]:os can not support static allocation.\nerror in file: ")+String("file:")+String(__FILE__)+String(",line:")+String(__LINE__,DEC));
    #endif
    esp_sleep_wakeup_cause_t cause =  esp_sleep_get_wakeup_cause();
 
    switch(cause){
        case ESP_SLEEP_WAKEUP_UNDEFINED:break;    //!< In case of deep sleep, reset was not caused by exit from deep sleep
        case ESP_SLEEP_WAKEUP_ALL:break;           //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
        case ESP_SLEEP_WAKEUP_EXT0:break;          //!< Wakeup caused by external signal using RTC_IO
        case ESP_SLEEP_WAKEUP_EXT1:break;         //!< Wakeup caused by external signal using RTC_CNTL
        case ESP_SLEEP_WAKEUP_TIMER:break;         //!< Wakeup caused by timer
        case ESP_SLEEP_WAKEUP_TOUCHPAD:break;      //!< Wakeup caused by touchpad
        case ESP_SLEEP_WAKEUP_ULP:break;           //!< Wakeup caused by ULP program
        case ESP_SLEEP_WAKEUP_GPIO:break;         //!< Wakeup caused by GPIO (light sleep only)
        case ESP_SLEEP_WAKEUP_UART:break; 
    }
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,0);
    //uint64_t mask = 1|1<<26;
    //esp_sleep_enable_ext1_wakeup(mask,ESP_EXT1_WAKEUP_ANY_HIGH);
     //gpio_wakeup_enable(GPIO_NUM_0,GPIO_INTR_POSEDGE)
    //gpio_wakeup_enable(GPIO_NUM_26,GPIO_INTR_POSEDGE)
    String mac_address=WiFi.macAddress();
    mac_address.replace(":","");
    platformio_api::get_web_properties().ap_ssid = WiFi.macAddress();
    PlatformDebug::println("DeviceInfo::BoardID:"+mac_address);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    unsigned int len= mac_address.length();
    platformio_api::get_device_info().BoardID = mac_address.substring(len-4,len);
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

    TracePrinter::init();

    LED::io_state_reset();

    //attachInterrupt(0,&guard::LoopTaskGuard::set_signal_web_service,FALLING);
    
    TracePrinter::printTrace("\n--- "+String(__DATE__)+" "+String(__TIME__)+" ---\n");
}

void IPSBox::i2c_devices_init()
{
    //static DS1307Builder b(_mtx_i2c_bus,Wire1);
    //bool  ok = RTC::BuildSchedule(b);
    /*
    MCP::BuildDigitalInOutPipe(MCP23017Builder(_mtx_i2c_bus,Wire1));

    MCP::PortOut::write(MCP23017Port::A,(uint8_t)~0b11111100);
    MCP::PortOut::write(MCP23017Port::B,(uint8_t)~0b11110111);
	delay(200);
	MCP::PortOut::write(MCP23017Port::A,0b11111100);
    MCP::PortOut::write(MCP23017Port::B,0b11110111);

    delay(200);
    MCP::DigitalOut::write(PinName::GPA1,1);
    delay(200);
    MCP::DigitalOut::write(PinName::GPB3,1);
    delay(200);
    MCP::DigitalOut::write(PinName::GPA1,0);
    delay(200);
    MCP::DigitalOut::write(PinName::GPB3,0);
    */
    bool  ok = RTC::BuildSchedule(DS3231MBuilder(_mtx_i2c_bus,Wire1));
    
    LED::io_state(LedName::RTC,ok);    

    String tm,tm2;
    RTC::UniversalTime::now(tm2);
    TracePrinter::printTrace("UniversalTime:"+tm2);
    delay(100);
    RTC::LocalDate::now(tm);
    TracePrinter::printTrace("LocalDate:"+tm);
    delay(100);
    RTC::LocalTime::now(tm);
    TracePrinter::printTrace("LocalTime:"+tm);
    delay(100);
    RTC::LocalDateTime::now(tm2);
    TracePrinter::printTrace("LocalDateTime:"+tm2);
    delay(100);
    RTC::ZonedDateTime::now(tm2);
    TracePrinter::printTrace("ZonedDateTime:"+tm2);
    delay(100);
    SystemClock::UniversalTime::now(tm2);
    TracePrinter::printTrace("SystemClockTime:"+tm2);

    DynamicJsonDocument doc(3000);
    ArduinoJson::JsonArray data=doc.createNestedArray("error_functions");
    Logger::getInstance().error_log_get(data);
    for(auto v : data){
        TracePrinter::printTrace(v.as<String>());
    }
}

void IPSBox::lora_gateway_init()
{
    _topics.push_back(platformio_api::get_device_info().Family+ String("/command/request/GW"));

    _loRaService.addOnMessageCallback(mbed::callback(&_loRaGateway,&LoRaGateway::onLoRaMessageCallback));
    _asyncMqttClientService.addOnMqttMessageCallback(mbed::callback(&_loRaGateway,&LoRaGateway::onMqttMessageCallback));
    
   
    _loRaGateway.setupBeacons(); 
    _loRaGateway.startup();

    _asyncMqttClientService.startup();
    _wifiService.addOnWiFiServiceCallback(mbed::callback(this,&IPSBox::onWiFiServiceCallback));
    _wifiService.startup();

    _asyncMqttClientService.addOnMqttDisonnectCallback(mbed::callback(this,&IPSBox::onMqttDisconnectCallback));
    _asyncMqttClientService.addOnMqttConnectCallback(mbed::callback(this,&IPSBox::onMqttConnectCallback));
    _loRaService.startup();

    
}
void IPSBox::lora_collector_init()
{
  
    _topics.push_back(platformio_api::get_device_info().Family+ String("/command/request/DC"));
    _topics.push_back(platformio_api::get_device_info().Family+String("/send_rssi/#"));
   
    _loRaService.addOnMessageCallback(mbed::callback(&_loRaCollector,&LoRaCollector::onLoRaMessageCallback));
    _asyncMqttClientService.addOnMqttMessageCallback(mbed::callback(&_loRaCollector,&LoRaCollector::onMqttMessageCallback));
    
    _loRaCollector.setupBeacons();
    _loRaCollector.startup();

    _asyncMqttClientService.startup();
    _wifiService.addOnWiFiServiceCallback(mbed::callback(this,&IPSBox::onWiFiServiceCallback));
    _wifiService.startup();

    _asyncMqttClientService.addOnMqttDisonnectCallback(mbed::callback(this,&IPSBox::onMqttDisconnectCallback));
    _asyncMqttClientService.addOnMqttConnectCallback(mbed::callback(this,&IPSBox::onMqttConnectCallback));
    _loRaService.startup();
}
void IPSBox::lora_beacon_init()
{
    _topics.push_back(platformio_api::get_device_info().Family+ String("/command/request/BCN"));
    _loRaService.addOnMessageCallback(mbed::callback(&_loRaBeacon,&LoRaBeacon::onLoRaMessageCallback));
    _asyncMqttClientService.addOnMqttMessageCallback(mbed::callback(&_loRaBeacon,&LoRaBeacon::onMqttMessageCallback));

    _loRaBeacon.startup(); 

    _asyncMqttClientService.startup();
    _wifiService.addOnWiFiServiceCallback(mbed::callback(this,&IPSBox::onWiFiServiceCallback));
    _wifiService.startup();

    _asyncMqttClientService.addOnMqttDisonnectCallback(mbed::callback(this,&IPSBox::onMqttDisconnectCallback));
    _asyncMqttClientService.addOnMqttConnectCallback(mbed::callback(this,&IPSBox::onMqttConnectCallback));
    _loRaService.startup();
}
void IPSBox::lora_tag_init()
{
    
    _loRaService.addOnMessageCallback(mbed::callback(&_loraTag,&LoRaTag::onLoRaMessageCallback));
    _loRaService.startup();

    _loraTag.startup();
    _loraTag.post_mail();
}

void IPSBox::onMqttMessageCallback(const String& topic,const String& payload)
{
    
}

void IPSBox::onMqttSubscribeCallback(uint16_t packetId, uint8_t qos)
{

}
void IPSBox::onMqttConnectCallback(bool sessionPresent)
{
  
    _boot_flipper.detach();
    for(auto& v : _topics){
        _asyncMqttClientService.subscribe(v);
    }
}
void IPSBox::onMqttDisconnectCallback(AsyncMqttClientDisconnectReason reason)
{

    _boot_flipper.attach(mbed::callback(this,&IPSBox::restart), std::chrono::seconds(30));
    if(_wifiService.isConnected()){
         _asyncMqttClientService.connect();
    }
}


void  IPSBox::start_web_service()
{   
  if(!_espWebService.isRunning()){

    LED::io_state(LedName::WEB,true);
    _asyncMqttClientService.cleanupCallbacks();
    _asyncMqttClientService.shutdown();
    _asyncMqttClientService.disconnect();

    _wifiService.cleanupCallbacks();
    _wifiService.shutdown();

    _boot_flipper.detach();

    _espWebService.startup();  

    _wifiService.switch_wifi_mode_to_AP();
    _espWebService.set_start_signal();

    _loraTag.shutdown();
    _espWebService.startup();  

    _wifiService.switch_wifi_mode_to_AP();
    _espWebService.set_start_signal();
  }
}