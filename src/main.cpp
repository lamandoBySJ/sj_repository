#include <heltec.h>
#include <app/TimeMachine/TimeMachine.h>
#include <app/ColorSensor/ColorSensor.h>
#include <ColorSensorBase.h>
#include <SPI.h>
#include <MFRC522.h>
#include <chrono>
#include <esp_event_legacy.h>
#include <WiFiType.h>
#include <WiFi.h>
#include "app/OLEDScreen/OLEDScreen.h"
#include "rtos/cmsis_os2.h"
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
  #include "freertos/task.h"
  #include "freertos/queue.h"
}
#include "rtos/Queue.h"
#include "rtos/Mail.h"
#include "rtos/MemoryPool.h"
#include <app/AsyncMqttClient/AsyncMqttClient.h>

#include "MQTTNetwork.h"
#include "LoRaNetwork.h"
#include "LoRaGateway.h"
#include "LoRaCollector.h"
#include "LoRaBeacon.h"
#include "esp_sleep.h"
#include <cxxsupport/mstd_new.h>
#include <new>
#include <mutex>
#include <thread>

#include "FS.h"
#include "FFat.h"
#include "FFatHelper.h"
#include "rtos/Mutex.h"
#include "rtos/cmsis_os2.h"
#include "freertos/queue.h"
#include "platform_debug.h"
#include "SmartBox.h"
using namespace std;
using namespace mstd;
using namespace rtos;
using namespace platform_debug;
#define BAND    470E6 
//python3 esptool.py --port COM20 --baud 115200 write_flash -fm dio -fs 16MB  0x410000 partitions.bin
//C:\Users\Administrator\.platformio\packages\framework-arduinoespressif32\tools\partitions
//DS1307 ds1307(Wire1,32,33); //ips

//#define OLEDSCREEN 
void setup() {
 // put your setup code here, to run once:
  #ifdef NDEBUG
    //WIFI Kit series V1 not support Vext control
    Heltec.begin(false, true , false , true, BAND);
    Serial.setDebugOutput(false);
    //LoRa.dumpRegisters(Serial);
  #else
    #ifdef OLEDSCREEN
      Heltec.begin(true, true , true , true, BAND);
       // PlatformDebug::init(Serial,oled);
        PlatformDebug::init(Serial,OLEDScreen<12>(Heltec.display));
        // PlatformDebug::init(Serial,std::move(oled));
        PlatformDebug::printLogo();
    #else
      Heltec.begin(false, false , true , true, BAND);
      PlatformDebug::getInstance()->init(Serial);
    #endif
  #endif

  TracePrinter::getInstance()->startup();

  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  platform_debug::PlatformDebug::println(" ************ STLB ************ ");

   esp_sleep_wakeup_cause_t cause =  esp_sleep_get_wakeup_cause();
   switch(cause){
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_UNDEFINED");
    break;    //!< In case of deep sleep, reset was not caused by exit from deep sleep
    case ESP_SLEEP_WAKEUP_ALL:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_ALL");
    break;           //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
    case ESP_SLEEP_WAKEUP_EXT0:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_EXT0");
    break;          //!< Wakeup caused by external signal using RTC_IO
    case ESP_SLEEP_WAKEUP_EXT1:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_EXT1");
    break;         //!< Wakeup caused by external signal using RTC_CNTL
    case ESP_SLEEP_WAKEUP_TIMER:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_TIMER");
    break;         //!< Wakeup caused by timer
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_TOUCHPAD");
    break;      //!< Wakeup caused by touchpad
    case ESP_SLEEP_WAKEUP_ULP:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_ULP");
    break;           //!< Wakeup caused by ULP program
    case ESP_SLEEP_WAKEUP_GPIO:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_GPIO");
    break;         //!< Wakeup caused by GPIO (light sleep only)
    case ESP_SLEEP_WAKEUP_UART:
    platform_debug::TracePrinter::printTrace("ESP_SLEEP_WAKEUP_UART");
    break; 
    default:break;
  }
  
  String mac_address=WiFi.macAddress();
  mac_address.replace(":","");
  Platform::getWebProperties()->ap_ssid = WiFi.macAddress();
  platform_debug::PlatformDebug::println("DeviceInfo::BoardID:"+mac_address);
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  Platform::getDeviceInfo()->BoardID = mac_address;
  Platform::getDeviceInfo()->Family = "k49a";
  platform_debug::PlatformDebug::println(Platform::getDeviceInfo()->BoardID);


  if(FFatHelper::init()){
      platform_debug::PlatformDebug::println("OK:File system mounted");
  }else{
      platform_debug::PlatformDebug::println("ERROR:File system:");
  }

  //FFatHelper::deleteFiles(FFat, "/", 0);
  FFatHelper::listDir(FFat, "/", 0);
  if(!FFatHelper::exists("/TowerColor")){
      FFatHelper::createDir(FFat,"/TowerColor");
      platform_debug::PlatformDebug::println("Dir created:/TowerColor");
  }
  if(!FFatHelper::exists("/data")){
      FFatHelper::createDir(FFat,"/data");
      platform_debug::PlatformDebug::println("Dir created:/data");
  }
  

  platform_debug::PlatformDebug::println("user_properties::path:"+Platform::getUserProperties()->path);
  platform_debug::PlatformDebug::println("user_properties::ssid:"+Platform::getUserProperties()->ssid);
  platform_debug::PlatformDebug::println("user_properties::pass:"+Platform::getUserProperties()->pass);
  platform_debug::PlatformDebug::println("user_properties::host:"+Platform::getUserProperties()->host);
  platform_debug::PlatformDebug::println(String(Platform::getUserProperties()->port,DEC));

 
  DynamicJsonDocument  docProperties(1024);
  String text;
 if(FFatHelper::readFile(FFat,Platform::getUserProperties()->path,text)){
      platform_debug::PlatformDebug::println(text); 
      DeserializationError error = deserializeJson(docProperties, text);
      if(!error){
          Platform::getUserProperties()->ssid =  docProperties["ssid"].as<String>();
          Platform::getUserProperties()->pass =  docProperties["pass"].as<String>();
          Platform::getUserProperties()->host =  docProperties["host"].as<String>();
          Platform::getUserProperties()->port =  docProperties["port"].as<int>();
          
          platform_debug::PlatformDebug::println("user_properties::ssid:"+Platform::getUserProperties()->ssid);
          platform_debug::PlatformDebug::println("user_properties::pass:"+Platform::getUserProperties()->pass);
          platform_debug::PlatformDebug::println("user_properties::host:"+Platform::getUserProperties()->host);
          platform_debug::PlatformDebug::println("user_properties::port:"+String(Platform::getUserProperties()->port,DEC));
      }
  }

  if(FFatHelper::readFile(FFat,ColorCollector::rgb_properties.path,text)){
      DeserializationError error= deserializeJson(docProperties, text);
      if(!error){
          ColorCollector::rgb_properties.r_offset =  docProperties["r_offset"].as<uint16_t>();
          ColorCollector::rgb_properties.g_offset =  docProperties["g_offset"].as<uint16_t>();
          ColorCollector::rgb_properties.b_offset =  docProperties["b_offset"].as<uint16_t>();
          platform_debug::PlatformDebug::println("rgb_properties::r_offset:"+String(ColorCollector::rgb_properties.r_offset));
          platform_debug::PlatformDebug::println("rgb_properties::g_offset:"+String(ColorCollector::rgb_properties.g_offset));
          platform_debug::PlatformDebug::println("rgb_properties::b_offset:"+String(ColorCollector::rgb_properties.b_offset));
      }
  }
 
  platform_debug::TracePrinter::printTrace("\n---------------- "+String(__DATE__)+" "+String(__TIME__)+" ----------------\n");
  
}

SmartBox smartBox;
os::ThreadControlGuard threadControlGuard;
void loop() {
  
 attachInterrupt(0,[&](){
    detachInterrupt(0);
    threadControlGuard.set_signal_id(0,true);
  },FALLING);
 
  smartBox.startup();

  while (true) {
    //ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));    
     switch (threadControlGuard.get_signal_id())
     {
     case 0:
        smartBox.start_web_task();
       break;
     case 1:
      break;
     case 2:
      break;
     case 3:
      break;
     default:
      break;
    }
  }
}
/*
if( timeMachine.getDateTime(currentTime)){
            platform_debug::TracePrinter::printTrace(currentTime);
}else{
            platform_debug::TracePrinter::printTrace("ERROR:currentTime");
}
bool ret =  MQTTNetwork::getClient()->publish("test",currentTime);
if(!ret){
            platform_debug::TracePrinter::printTrace("publish Fail");
}
*/
//platform_debug::PlatformDebug::pause();
//std::this_thread::sleep_for(chrono::seconds(10));
//  std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
//  lck.lock();
// std::lock_guard<rtos::Mutex> lck(_mtx);
