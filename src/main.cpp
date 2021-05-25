#include <heltec.h>
#include <app/TimeMachine/TimeMachine.h>
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
#include "LoopTaskGuard.h"


using namespace std;
#define BAND    470E6 
//python3 esptool.py --port COM20 --baud 115200 write_flash -fm dio -fs 16MB  0x410000 partitions.bin
//C:\Users\Administrator\.platformio\packages\framework-arduinoespressif32\tools\partitions
//DS1307 ds1307(Wire1,32,33); //ips

std::mutex std_mutex;
rtos::Mutex stdMutex;


SmartBox* smartBox=new SmartBox();

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

  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  PlatformDebug::println(" ************ STLB ************ ");

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
  Platform::getWebProperties().ap_ssid = WiFi.macAddress();
  PlatformDebug::println("DeviceInfo::BoardID:"+mac_address);
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  Platform::getDeviceInfo().BoardID = mac_address;
  Platform::getDeviceInfo().Family = "k49a";
  PlatformDebug::println(Platform::getDeviceInfo().BoardID);

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
 if(FatHelper.readFile(FFat,Platform::getUserProperties().path,text)){
      PlatformDebug::println(text); 
      DeserializationError error = deserializeJson(docProperties, text);
      if(!error){
          Platform::getUserProperties().ssid =  docProperties["ssid"].as<String>();
          Platform::getUserProperties().pass =  docProperties["pass"].as<String>();
          Platform::getUserProperties().host =  docProperties["host"].as<String>();
          Platform::getUserProperties().port =  docProperties["port"].as<int>();
      }
  }

  if(FatHelper.readFile(FFat,Platform::getRGBProperties().path,text)){
      DeserializationError error= deserializeJson(docProperties, text);
      if(!error){
          Platform::getRGBProperties().r_offset =  docProperties["r_offset"].as<uint16_t>();
          Platform::getRGBProperties().g_offset =  docProperties["g_offset"].as<uint16_t>();
          Platform::getRGBProperties().b_offset =  docProperties["b_offset"].as<uint16_t>();
          PlatformDebug::println("rgb_properties::r_offset:"+String(Platform::getRGBProperties().r_offset));
          PlatformDebug::println("rgb_properties::g_offset:"+String(Platform::getRGBProperties().g_offset));
          PlatformDebug::println("rgb_properties::b_offset:"+String(Platform::getRGBProperties().b_offset));
      }
  }

  PlatformDebug::println("user_properties::path:"+Platform::getUserProperties().path);
  PlatformDebug::println("user_properties::ssid:"+Platform::getUserProperties().ssid);
  PlatformDebug::println("user_properties::pass:"+Platform::getUserProperties().pass);
  PlatformDebug::println("user_properties::host:"+Platform::getUserProperties().host);
  PlatformDebug::println(String(Platform::getUserProperties().port,DEC));

  TracePrinter::getInstance()->startup();
  TracePrinter::printTrace("\n---------------- "+String(__DATE__)+" "+String(__TIME__)+" ----------------\n");
  
}


void loop() {
  guard::LoopTaskGuard::getLoopTaskGuard();

  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1)); 
  attachInterrupt(0,[&](){
    //detachInterrupt(0);
    guard::LoopTaskGuard::getLoopTaskGuard().set_signal_id(0,true);
  },FALLING);

  smartBox->startup();

  while (true) {

    switch (guard::LoopTaskGuard::getLoopTaskGuard().get_signal_id())
    {
     case 0:
          {
            guard::LoopTaskGuard::getLoopTaskGuard().loop_stop();
            smartBox->task_web_service();
          }
       break;
     case 1:
          {
              smartBox->color_measure();
          }
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
//PlatformDebug::pause();
//std::this_thread::sleep_for(chrono::seconds(10));
//  std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
//  lck.lock();
// std::lock_guard<rtos::Mutex> lck(_mtx);
