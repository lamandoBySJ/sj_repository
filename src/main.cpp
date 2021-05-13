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
#include "platform_debug.h"
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
#include <functional>
#include <HTTPClient.h>
#include <Esp32httpUpdate.h>
#include "FS.h"
#include "FFat.h"
#include "FFatHelper.h"
#include "ESPwebServer.h"
#include "RGBCollector.h"
#include "OTAService.h"
#include "CmdParser.h"
#include "rtos/Mutex.h"
#include "rtos/cmsis_os2.h"
#include "freertos/queue.h"

using namespace std;
using namespace mstd;
using namespace rtos;
using namespace platform_debug;

#define BAND    470E6 
#if CONFIG_AUTOSTART_ARDUINO
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#endif
//python3 esptool.py --port COM20 --baud 115200 write_flash -fm dio -fs 16MB  0x410000 partitions.bin
//C:\Users\Administrator\.platformio\packages\framework-arduinoespressif32\tools\partitions

// mutex for critical section
std::mutex mtx; 
DS1307 ds1307(Wire1,21,22); //stlb
//DS1307 ds1307(Wire1,32,33); //ips
TimeMachine<DS1307> timeMachine(ds1307,mtx,13);  

BH1749NUC bh1749nuc(Wire,4,15);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,mtx,2);

CmdParser cmdParser;
MQTTNetwork MQTTnetwork;
//OLEDScreen<12> oled(Heltec.display);
ESPWebServer ESPwebServer;
RGBCollector<BH1749NUC> RGBcollector(MQTTnetwork,colorSensor);
//OLEDScreen<12> oled(Heltec.display);


struct mail_control_t{
  uint32_t counter=0; 
  uint32_t id=0;   
};
rtos::Mail<mail_control_t, 16> mail_box;
Thread threadWeb(osPriorityNormal,1024*6,NULL,"web");
Thread threadSignal(osPriorityNormal,1024*6,NULL,"signal");

volatile bool flag=true;
class Test
{
public:
  void run(){
    while(flag){
      Serial.println("Testing......"+String(ThisThread::flags_wait_any_for(0x4,std::chrono::seconds(60),true),DEC));
      //std::this_thread::sleep_for(std::chrono::seconds(3));
    }
  }
  void siganl(){
    while(1){
      osEvent evt=  mail_box.get();
      if (evt.status == osEventMail) {
        /*if(!ESPwebServer.isRunning()){

          threadWeb.start(callback(&ESPwebServer,&ESPWebServer::startup));
          threadWeb.join();
        }*/
        mail_control_t* mail= (mail_control_t*)evt.value.p;
        threadWeb.flags_set(4);
        Serial.println("set:"+String(mail->id,DEC));
        mail_box.free(mail);
      }
    }
  }
};

Test test;
std::thread thd;
//Thread threads[2];
//#define OLEDSCREEN 
void setup() {
 // put your setup code here, to run once:
  //WIFI Kit series V1 not support Vext control
  esp_sleep_wakeup_cause_t cause =  esp_sleep_get_wakeup_cause();
  #ifdef NDEBUG
    Heltec.begin(false, true , false , true, BAND);
    Serial.setDebugOutput(false);
  #else
    #ifdef OLEDSCREEN
      Heltec.begin(true, true , true , true, BAND);
     // PlatformDebug::init(Serial,oled);
      PlatformDebug::init(Serial,OLEDScreen<12>(Heltec.display));
     // PlatformDebug::init(Serial,std::move(oled));
      PlatformDebug::printLogo();
     
    #else
      Heltec.begin(false, false , true , true, BAND);
      PlatformDebug::init(Serial);
    #endif
  #endif
     //thd = std::thread(&Test::terminal,&test);
    // thd.detach();
 
  attachInterrupt(0,[](){
    volatile static int id=0;
    mail_control_t* mail=  mail_box.alloc();
    mail->id = ++id;
    mail_box.put_from_isr(mail);
  },FALLING);
  //thd = std::thread(&Test::run,&test);
  Serial.println("ThreadStart");
  threadWeb.start(callback(&test,&Test::run));
  threadSignal.start(callback(&test,&Test::siganl));

  threadWeb.join();
  threadSignal.join();

  platform_debug::PlatformDebug::pause();
  //LoRa.dumpRegisters(Serial);
 //   SemaphoreHandle_t hMutex;
 // vQueueAddToRegistry(hMutex,"");
   TracePrinter::startup();

  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  platform_debug::PlatformDebug::println(" ************ STLB ************ ");
  
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
  //threads[0].start(callback(&test,&Test::run));
 // threads[1].start(callback(&test,&Test::run));
 // for(;;){
 //   static int i=0;
 //   platform_debug::TracePrinter::printTrace("1111111111111111111111111"+String(++i,DEC));
  //  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //}
  
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  pinMode(22,PULLUP);
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,0);
  //uint64_t mask = 1|1<<26;
  //esp_sleep_enable_ext1_wakeup(mask,ESP_EXT1_WAKEUP_ANY_HIGH);
  //gpio_wakeup_enable(GPIO_NUM_0,GPIO_INTR_LOW_LEVEL);
  //gpio_wakeup_enable(GPIO_NUM_26,GPIO_INTR_HIGH_LEVEL);
  //esp_sleep_enable_gpio_wakeup();
 /*
  //int Maxint = numeric_limits<int32_t>::max();
  //int Minint = numeric_limits<int32_t>::min();
  //platform_debug::PlatformDebug::println(" ************ Max ************ "+String(Maxint,DEC));
  //platform_debug::PlatformDebug::println(" ************ Min ************ "+String(Minint,DEC));
  pinMode(0, PULLUP);
  */
 
  

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

  web_properties::ap_ssid = WiFi.macAddress();
  platform_debug::DeviceInfo::BoardID = mac_address.c_str();
  //platform_debug::DeviceInfo::BoardID = String(mac_address.substr(mac_address.length()-4,4).c_str());
  platform_debug::PlatformDebug::println("DeviceInfo::BoardID:"+platform_debug::DeviceInfo::BoardID);
  


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

  DynamicJsonDocument  docProperties(1024);
  String text;
  if(FFatHelper::readFile(FFat,user_properties::path,text)){
      platform_debug::PlatformDebug::println(text); 
      DeserializationError error = deserializeJson(docProperties, text);
      if(!error){
          user_properties::ssid =  docProperties["ssid"].as<String>();
          user_properties::pass =  docProperties["pass"].as<String>();
          user_properties::host =  docProperties["host"].as<String>();
          user_properties::port =  docProperties["port"].as<int>();
          
          platform_debug::PlatformDebug::println("user_properties::ssid:"+user_properties::ssid);
          platform_debug::PlatformDebug::println("user_properties::pass:"+user_properties::pass);
          platform_debug::PlatformDebug::println("user_properties::host:"+user_properties::host);
          platform_debug::PlatformDebug::println("user_properties::port:"+String(user_properties::port,DEC));
      }
  }

  if(FFatHelper::readFile(FFat,rgb_properties::path,text)){
      DeserializationError error= deserializeJson(docProperties, text);
      if(!error){
          rgb_properties::r_offset =  docProperties["r_offset"].as<uint16_t>();
          rgb_properties::g_offset =  docProperties["g_offset"].as<uint16_t>();
          rgb_properties::b_offset =  docProperties["b_offset"].as<uint16_t>();
          platform_debug::PlatformDebug::println("rgb_properties::r_offset:"+String(rgb_properties::r_offset));
          platform_debug::PlatformDebug::println("rgb_properties::g_offset:"+String(rgb_properties::g_offset));
          platform_debug::PlatformDebug::println("rgb_properties::b_offset:"+String(rgb_properties::b_offset));
      }
  }
 
//  timeMachine.startup(true,__DATE__,__TIME__);
//timeMachine.setEpoch(1614764209+8*60*60);
 // colorSensor.startup();
  //MQTTnetwork.addTopic("SmartBox/TimeSync");
  MQTTnetwork.addSubscribeTopic(platform_debug::DeviceInfo::BoardID+"/ServerTime");
  MQTTnetwork.addSubscribeTopic(platform_debug::DeviceInfo::BoardID+"/ServerReq");
  MQTTnetwork.addOnMessageCallback(callback(&cmdParser,&CmdParser::onMessageCallback));

  
  ESPwebServer.setCallbackPostMailToCollector(callback(&RGBcollector,&RGBCollector<BH1749NUC>::delegateMethodPostMail));
  RGBcollector.setWebSocketClientEventCallback(callback(&ESPwebServer,&ESPWebServer::delegateMethodWebSocketClientPostEvent));
  RGBcollector.setWebSocketClientTextCallback(callback(&ESPwebServer,&ESPWebServer::delegateMethodWebSocketClientText));
  RGBcollector.startup();

  MQTTnetwork.startup();
  platform_debug::TracePrinter::printTrace("\n---------------- "+String(__DATE__)+" "+String(__TIME__)+" ----------------\n");
 //platform_debug::PlatformDebug::pause();
}

RGB rgb;
static String currentTime="";

void loop() {
  pinMode(21,OUTPUT);
  pinMode(23,OUTPUT);
//  digitalWrite(21,HIGH);
//  digitalWrite(23,HIGH);

 
  while (true) {
   // _std_mutex.lock();
   // if( timeMachine.getDateTime(currentTime)){
   //     platform_debug::PlatformDebug::println(currentTime);
   // }else{
   //     platform_debug::PlatformDebug::println("ERROR:currentTime");
   // }
   // _std_mutex.unlock();
   // RGBcollector.delegateMethodPostMail(MeasEventType::EventSystemMeasure);
    
    //ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(3000));
    
    osEvent evt=  mail_box.get();
    
    if (evt.status == osEventMail) {
      /*if(!ESPwebServer.isRunning()){

        threadWeb.start(callback(&ESPwebServer,&ESPWebServer::startup));
        threadWeb.join();
      }*/
      mail_control_t* mail= (mail_control_t*)evt.value.p;
      threadWeb.flags_set(mail->id);
      Serial.println("set:"+String(flag,DEC));
      mail_box.free(mail);
    }
    //std::this_thread::sleep_for(chrono::seconds(10));
  }
}



