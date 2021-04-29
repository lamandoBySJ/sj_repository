#include <heltec.h>
#include <rtos/rtos.h>
#include <app/TimeMachine/TimeMachine.h>
#include <app/ColorSensor/ColorSensor.h>
#include <ColorSensorBase.h>
#include <SPI.h>
#include <MFRC522.h>
#include <chrono>
#include <DelegateClass.h>

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
<<<<<<< HEAD
#include "FFatHelper.h"
=======
#include <cxxsupport/mstd_new.h>
#include <new>
#include <mutex>
#include <thread>
#include <functional>
#include <Test.h>

#include <HTTPClient.h>
#include <httpUpdate.h>
#include <Esp32httpUpdate.h>
#include "FS.h"
#include "FFat.h"
#include "FFatHelper.h"

using namespace std;
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
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

<<<<<<< HEAD
String beacon_properties::path="/beacon_properties";
=======
std::mutex mtx;           // mutex for critical section
rtos::Mutex std_mutex;
//DS1307 ds1307(Wire1,21,22); //stlb
DS1307 ds1307(Wire1,32,33); //ips
TimeMachine<DS1307> timeMachine(ds1307,mtx,13);  
//TimeMachine<RTCBase> timeMachine(&RTC,std_mutex);

BH1749NUC bh1749nuc(Wire,4,15);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,mtx,2);
//ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8

String DeviceInfo::BoardID="";
String DeviceInfo::Family="k49a";
OLEDScreen<12> oled(Heltec.display);
TracePrinter tracePrinter;
<<<<<<< HEAD

=======
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
MQTTNetwork mqttNetwork;
//Test t;
using namespace thread_test;
using namespace esp32_http_update;
//#define OLEDSCREEN 1
bool update = false;
void setup() {
 // put your setup code here, to run once:
  //WIFI Kit series V1 not support Vext control
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
 
  //gpio_wakeup_enable(GPIO_NUM_0,GPIO_INTR_LOW_LEVEL);
  //gpio_wakeup_enable(GPIO_NUM_26,GPIO_INTR_HIGH_LEVEL);
  //esp_sleep_enable_gpio_wakeup();

  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  pinMode(22,PULLUP);

  #ifdef NDEBUG
    Heltec.begin(false, true , false , true, BAND);
  #else
    #ifdef OLEDSCREEN
      Heltec.begin(true, true , true , true, BAND);
      PlatformDebug::init(Serial,OLEDScreen<12>(Heltec.display));
      PlatformDebug::printLogo();
    #else
      Heltec.begin(false, false , true , true, BAND);
      PlatformDebug::init(Serial);
    #endif
  #endif
  //LoRa.dumpRegisters(Serial);
  //PlatformDebug::init(Serial);
  //PlatformDebug::init(Serial,oled);
  //PlatformDebug::init(oled);
  //PlatformDebug::init(OLEDScreen<12>(Heltec.display));
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  platform_debug::PlatformDebug::println(" ************ IPS ************ ");
  pinMode(0, PULLUP);
  attachInterrupt(0,[]()->void{
      update = true;
  },FALLING);
  
  tracePrinter.startup();
  
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

  platform_debug::DeviceInfo::BoardID = String(mac_address.substr(mac_address.length()-4,4).c_str());
  platform_debug::PlatformDebug::println("DeviceInfo::BoardID:"+platform_debug::DeviceInfo::BoardID);
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  
  if(FFatHelper::init()){
      platform_debug::PlatformDebug::println("FFat Mount OK");
  }else{
     platform_debug::PlatformDebug::println("FFat Mount Failed");
  }

  String text ="";
  if(!FFatHelper::readFile(FFat,beacon_properties::path,text)){
      text="{\"beacons\":[{\"15BC\":\"a\"},{\"15E8\":\"b\"},{\"1570\":\"c\"},{\"16BC\":\"d\"}]}";
  }



  timeMachine.startup();
  //timeMachine.setEpoch(1614764209+8*60*60);
 // colorSensor.startup();
  //mqttNetwork.addTopics(loRaCollector.getTopics());
  //mqttNetwork.addOnMessageCallback(callback(&loRaCollector,&LoRaCollector::onMessageMqttCallback));
  //mqttNetwork.addOnMqttConnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttConnectCallback));
  //mqttNetwork.addOnMqttDisonnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttDisconnectCallback));
  
  mqttNetwork.startup();

<<<<<<< HEAD
  //colorSensor.startup();
  /*
  loRaNetwork.addOnMessageCallback(callback(&loRaGateway,&LoRaGateway::onMessageLoRaCallback));
  loRaGateway.setupBeacons(text);
  loRaGateway.startup();
  mqttNetwork.addTopics(loRaGateway.getTopics());
  mqttNetwork.addOnMessageCallback(callback(&loRaGateway,&LoRaGateway::onMessageMqttCallback));
  mqttNetwork.addOnMqttConnectCallback(callback(&loRaGateway,&LoRaGateway::onMqttConnectCallback));
  mqttNetwork.addOnMqttDisonnectCallback(callback(&loRaGateway,&LoRaGateway::onMqttDisconnectCallback));
 // */

///*
  loRaNetwork.addOnMessageCallback(callback(&loRaCollector,&LoRaCollector::onMessageLoRaCallback));
  loRaCollector.setupBeacons(text);
  loRaCollector.startup();
  mqttNetwork.addTopics(loRaCollector.getTopics());
  mqttNetwork.addOnMessageCallback(callback(&loRaCollector,&LoRaCollector::onMessageMqttCallback));
  mqttNetwork.addOnMqttConnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttConnectCallback));
  mqttNetwork.addOnMqttDisonnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttDisconnectCallback));
 // */

  /*
  loRaNetwork.addOnMessageCallback(callback(&loRaBeacon,&LoRaBeacon::onMessageLoRaCallback));
  loRaBeacon.startup();
  mqttNetwork.addTopics(loRaBeacon.getTopics());
  mqttNetwork.addOnMessageCallback(callback(&loRaBeacon,&LoRaBeacon::onMessageMqttCallback));
  mqttNetwork.addOnMqttConnectCallback(callback(&loRaBeacon,&LoRaBeacon::onMqttConnectCallback));
  mqttNetwork.addOnMqttDisonnectCallback(callback(&loRaBeacon,&LoRaBeacon::onMqttDisconnectCallback));
  //*/
  
  mqttNetwork.startup();
  loRaNetwork.startup();
=======
  //std::thread threads[2];
  //threads[0]= std::thread(print_thread_id_test, 1);
  //threads[1]= std::thread(print_thread_id_test2, 2);

  // test_unique_lock_cd();
 
  // while(1){  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));};
  platform_debug::PlatformDebug::println(">>>>>>>>>>>>>>>> Setup over >>>>>>>>>>>>>>>>>>");
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
  

  if(FFatHelper::init()){
      platform_debug::PlatformDebug::println("OK:File system mounted");
  }else{
      platform_debug::PlatformDebug::println("ERROR:File system:");
  }

  platform_debug::PlatformDebug::println("---------------- "+String(__TIME__)+" ----------------");
}

RGB rgb;
static String currentTime="";
void loop() {

  while (true) {
    if( timeMachine.getDateTime(currentTime)){
        platform_debug::PlatformDebug::println(currentTime);
    }else{
        platform_debug::PlatformDebug::println("ERROR:currentTime");
    }
    
     if(update){
       platform_debug::PlatformDebug::println("Start:OTA...");
       esp32_http_update::t_httpUpdate_return ret =  esp32_http_update::ESPhttpUpdate.update("http://192.168.1.100/bin/firmware.bin");

        switch(ret) {
            case esp32_http_update::HTTP_UPDATE_FAILED:
                platform_debug::PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                break;
            case esp32_http_update::HTTP_UPDATE_NO_UPDATES:
               platform_debug::PlatformDebug::println("HTTP_UPDATE_NO_UPDATES");
                break;
            case esp32_http_update::HTTP_UPDATE_OK:
                platform_debug::PlatformDebug::println("HTTP_UPDATE_OK");
                std::this_thread::sleep_for(chrono::seconds(3));
                ESP.restart();
                break;
        }
     }
    std::this_thread::sleep_for(chrono::seconds(3));
    //ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(3000));
  }
}



