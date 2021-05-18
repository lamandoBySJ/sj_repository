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
#include "ESPWebService.h"
#include "ColorCollector.h"
#include "OTAService.h"
#include "CmdParser.h"
#include "rtos/Mutex.h"
#include "rtos/cmsis_os2.h"
#include "freertos/queue.h"
#include "platform_debug.h"

using namespace std;
using namespace mstd;
using namespace rtos;
using namespace platform_debug;
#define BAND    470E6 
//python3 esptool.py --port COM20 --baud 115200 write_flash -fm dio -fs 16MB  0x410000 partitions.bin
//C:\Users\Administrator\.platformio\packages\framework-arduinoespressif32\tools\partitions
//DS1307 ds1307(Wire1,32,33); //ips
std::mutex std_mutex;
TimeMachine<DS1307> timeMachine(std_mutex,21,22,13);  
TracePrinter* tracePrinter =nullptr;
//#define OLEDSCREEN 
void setup() {
 // put your setup code here, to run once:
  //WIFI Kit series V1 not support Vext control
 
  #ifdef NDEBUG
    Heltec.begin(false, true , false , true, BAND);
    Serial.setDebugOutput(false);
  #else
    #ifdef OLEDSCREEN
      Heltec.begin(true, true , true , true, BAND);
    #else
      Heltec.begin(false, false , true , true, BAND);
    #endif
  #endif

  platform_debug_init(true,false);
  tracePrinter = new  platform_debug::TracePrinter();
  tracePrinter->startup();
  
 // platform_debug::PlatformDebug::pause();
  //LoRa.dumpRegisters(Serial);
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
  
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,0);
  //uint64_t mask = 1|1<<26;
  //esp_sleep_enable_ext1_wakeup(mask,ESP_EXT1_WAKEUP_ANY_HIGH);
  //gpio_wakeup_enable(GPIO_NUM_0,GPIO_INTR_LOW_LEVEL);
  //gpio_wakeup_enable(GPIO_NUM_26,GPIO_INTR_HIGH_LEVEL);
  //esp_sleep_enable_gpio_wakeup();

  std::string mac_address=WiFi.macAddress().c_str();
  std::string mark=":";
  unsigned int nSize = mark.size();
  unsigned int position = mac_address.find(mark);
  if(position != string::npos){
     for(unsigned int pos=position;pos != string::npos;){
        mac_address.erase(pos, nSize);
        platform_debug::PlatformDebug::println(mac_address);
        pos= mac_address.find(mark);
    }
  }

  ESPWebService::webProperties.ap_ssid = WiFi.macAddress();
  Platform::deviceInfo.BoardID = mac_address.c_str();
  //Platform::deviceInfo.BoardID = String(mac_address.substr(mac_address.length()-4,4).c_str());
  platform_debug::PlatformDebug::println("DeviceInfo::BoardID:"+Platform::deviceInfo.BoardID);
  
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
  

  platform_debug::PlatformDebug::println("user_properties::path:"+MQTTNetwork::userProperties.path);
  platform_debug::PlatformDebug::println("user_properties::ssid:"+MQTTNetwork::userProperties.ssid);
  platform_debug::PlatformDebug::println("user_properties::pass:"+MQTTNetwork::userProperties.pass);
  platform_debug::PlatformDebug::println("user_properties::host:"+MQTTNetwork::userProperties.host);
  platform_debug::PlatformDebug::println(String(MQTTNetwork::userProperties.port,DEC));

 
  DynamicJsonDocument  docProperties(1024);
  String text;
 if(FFatHelper::readFile(FFat,MQTTNetwork::userProperties.path,text)){
      platform_debug::PlatformDebug::println(text); 
      DeserializationError error = deserializeJson(docProperties, text);
      if(!error){
          MQTTNetwork::userProperties.ssid =  docProperties["ssid"].as<String>();
          MQTTNetwork::userProperties.pass =  docProperties["pass"].as<String>();
          MQTTNetwork::userProperties.host =  docProperties["host"].as<String>();
          MQTTNetwork::userProperties.port =  docProperties["port"].as<int>();
          
          platform_debug::PlatformDebug::println("user_properties::ssid:"+MQTTNetwork::userProperties.ssid);
          platform_debug::PlatformDebug::println("user_properties::pass:"+MQTTNetwork::userProperties.pass);
          platform_debug::PlatformDebug::println("user_properties::host:"+MQTTNetwork::userProperties.host);
          platform_debug::PlatformDebug::println("user_properties::port:"+String(MQTTNetwork::userProperties.port,DEC));
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
  //platform_debug::PlatformDebug::pause();
}


//rtos::Thread thdDebug;
MQTTNetwork  MQTTnetwork;
void loop() {


  ColorCollector colorCollector;
  ESPWebService webService;
  
  timeMachine.startup(true,__DATE__,__TIME__);
//timeMachine.setEpoch(1614764209+8*60*60);

  MQTTnetwork.addSubscribeTopic("SmartBox/TimeSync");
  MQTTnetwork.addSubscribeTopic(Platform::deviceInfo.BoardID+"/ServerTime");
  MQTTnetwork.addSubscribeTopic(Platform::deviceInfo.BoardID+"/ServerReq");
  //MQTTnetwork.addOnMessageCallback(callback(&cmdParser,&CmdParser::onMessageCallback));

  webService.setCallbackPostMailToCollector(callback(&colorCollector,&ColorCollector::delegateMethodPostMail));
  colorCollector.setCallbackWebSocketClientEvent(callback(&webService,&ESPWebService::delegateMethodWebSocketClientEvent));
  colorCollector.setCallbackWebSocketClientText(callback(&webService,&ESPWebService::delegateMethodWebSocketClientText));
 
 //thdDebug.start(callback(&MQTTnetwork,&MQTTNetwork::runWiFiEventService));
  
  try{
      MQTTnetwork.startup();
      colorCollector.startup();
  }catch(const osStatusException& e){
     platform_debug::TracePrinter::printTrace(e.what());
     platform_debug::PlatformDebug::pause();
  }
  
   
  // webService.startup();
 /*attachInterrupt(0,[&](){
    volatile static int id=0;
    mail_control_t* mail=  mail_box_debug.alloc();
    mail->id = ++id;
    mail_box_debug.put_from_isr(mail);
  },FALLING);
  */
  String currentTime="";
  while (true) {

         ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));    
        if( timeMachine.getDateTime(currentTime)){
            platform_debug::TracePrinter::printTrace(currentTime);
         }else{
            platform_debug::TracePrinter::printTrace("ERROR:currentTime");
        }
       bool ret =  MQTTNetwork::getClient()->publish("test",currentTime);
      if(!ret){
          platform_debug::TracePrinter::printTrace("publish Fail");
      }
   // RGBcollector.delegateMethodPostMail(MeasEventType::EventSystemMeasure);
    //ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(3000));
  /*  rtos::Mail<mail_control_t, 16> mail_box_debug;
    
      osEvent evt=  mail_box_debug.get();
      if (evt.status == osEventMail) {
       if(!webService.isRunning()){

           webService.startup();
         // webServer.startup();
        }else{
          webService.shutdown();
        }
        mail_control_t* mail= (mail_control_t*)evt.value.p;
        mail_box_debug.free(mail);
      }*/
  }
}
//platform_debug::PlatformDebug::pause();
    //std::this_thread::sleep_for(chrono::seconds(10));
   //  std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
      //  lck.lock();
            // std::lock_guard<rtos::Mutex> lck(_mtx);
