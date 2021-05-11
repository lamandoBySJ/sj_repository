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
String rgb_properties::path ="/als_constant";
uint16_t rgb_properties::r_offset = 0;
uint16_t rgb_properties::g_offset = 0;
uint16_t rgb_properties::b_offset = 0;
String user_properties::path = "/user_constant";
String user_properties::ssid = "IoTwlan";
String user_properties::pass = "mitac1993";
String user_properties::host = "mslmqtt.mic.com.cn";
int    user_properties::port = 1883;

String web_properties::ap_ssid="STLB_SSID";
String web_properties::ap_pass="Aa000000";
String web_properties::http_user="admin";
String web_properties::http_pass="admin";
String web_properties::server_upload_uri = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
// mutex for critical section
std::mutex mtx; 
DS1307 ds1307(Wire1,21,22); //stlb
//DS1307 ds1307(Wire1,32,33); //ips
TimeMachine<DS1307> timeMachine(ds1307,mtx,13);  
//TimeMachine<RTCBase> timeMachine(&RTC,std_mutex);

BH1749NUC bh1749nuc(Wire,4,15);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,mtx,2);
//ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);

String DeviceInfo::BoardID="";
String DeviceInfo::Family="k49a";
//OLEDScreen<12> oled(Heltec.display);

MQTTNetwork MQTTnetwork;

bool update = false;
bool web_update= false;

ESPWebServer ESPwebServer;
RGBCollector<BH1749NUC> RGBcollector(MQTTnetwork,colorSensor);
OLEDScreen<12> oled(Heltec.display);
//OTAService OTAservice;
//CmdParser cmdParser;
// Mutex _std_mutex;
class Test
{
public:
  void run(){
     platform_debug::PlatformDebug::println(" ************ STLB ************ ");
     std::this_thread::sleep_for(std::chrono::seconds(1));
  }
};
#define OLEDSCREEN 
/*
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
  */

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
      PlatformDebug::init(Serial,std::move(oled));
      PlatformDebug::printLogo();
      TracePrinter::startup();
    #else
      Heltec.begin(false, false , true , true, BAND);
      PlatformDebug::init(Serial);
    #endif
  #endif
  //LoRa.dumpRegisters(Serial);
 //   SemaphoreHandle_t hMutex;
 // vQueueAddToRegistry(hMutex,"");
  
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
  Test test;
 std::thread thd= std::thread(&Test::run,&test);
  for(;;){
  //  platform_debug::TracePrinter::printTrace("1111111111111111111111111");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  
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
  attachInterrupt(0,[]()->void{
     
  },FALLING);
  */
/*
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
  web_properties::ap_ssid = String(WiFi.macAddress().c_str());
  platform_debug::DeviceInfo::BoardID = String(mac_address.c_str());
  //platform_debug::DeviceInfo::BoardID = String(mac_address.substr(mac_address.length()-4,4).c_str());
  platform_debug::PlatformDebug::println("DeviceInfo::BoardID:"+platform_debug::DeviceInfo::BoardID);
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));

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
  }else{
    docProperties.clear();
    docProperties["ssid"] = user_properties::ssid;
    docProperties["pass"] = user_properties::pass;
    docProperties["host"] = user_properties::host;
    docProperties["port"] = user_properties::port;
    FFatHelper::writeFile(FFat,user_properties::path,docProperties.as<String>());
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
  }else{
      docProperties.clear();
      docProperties["r_offset"] = rgb_properties::r_offset;
      docProperties["g_offset"] = rgb_properties::g_offset;
      docProperties["b_offset"] = rgb_properties::b_offset;
      FFatHelper::writeFile(FFat,rgb_properties::path,docProperties.as<String>());
  }
 
  timeMachine.startup(true,__DATE__,__TIME__);
//timeMachine.setEpoch(1614764209+8*60*60);
  colorSensor.startup();
  //MQTTnetwork.addTopic("SmartBox/TimeSync");
  MQTTnetwork.addSubscribeTopic(platform_debug::DeviceInfo::BoardID+"/ServerTime");
  MQTTnetwork.addSubscribeTopic(platform_debug::DeviceInfo::BoardID+"/ServerReq");

  //MQTTnetwork.addOnMessageCallback(callback(&cmdParser,&CmdParser::onMessageCallback));
  // MQTTnetwork.addOnMqttConnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttConnectCallback));
  // MQTTnetwork.addOnMqttDisonnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttDisconnectCallback));
  
  MQTTnetwork.startup();

  ESPwebServer.setCallbackPostMailToCollector(callback(&RGBcollector,&RGBCollector<BH1749NUC>::delegateMethodPostMail));
  RGBcollector.setWebSocketClientEventCallback(callback(&ESPwebServer,&ESPWebServer::delegateMethodWebSocketClientPostEvent));
  RGBcollector.setWebSocketClientTextCallback(callback(&ESPwebServer,&ESPWebServer::delegateMethodWebSocketClientText));
  RGBcollector.startup();
  ESPwebServer.startup();
  */

  platform_debug::TracePrinter::printTrace("\n---------------- "+String(__DATE__)+" "+String(__TIME__)+" ----------------\n");
}

RGB rgb;
static String currentTime="";

void loop() {

  while (true) {
   // _std_mutex.lock();
    if( timeMachine.getDateTime(currentTime)){
        platform_debug::PlatformDebug::println(currentTime);
    }else{
        platform_debug::PlatformDebug::println("ERROR:currentTime");
    }
   // _std_mutex.unlock();
   // RGBcollector.delegateMethodPostMail(MeasEventType::EventSystemMeasure);
    std::this_thread::sleep_for(chrono::seconds(10));
    //ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(3000));
    
  }
}



