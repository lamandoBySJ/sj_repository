#include <heltec.h>
#include <rtos/rtos.h>
#include <app/TimeMachine/TimeMachine.h>
#include <app/ColorSensor/ColorSensor.h>
#include <ColorSensorBase.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Test.h>
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


rtos::Mutex std_mutex;
DS1307 ds1307(Wire1,21,22);
TimeMachine<DS1307> timeMachine(ds1307,std_mutex,13);  
//TimeMachine<RTCBase> timeMachine(&RTC,std_mutex);

BH1749NUC bh1749nuc(Wire1,4,15);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,std_mutex,2);
//ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);

Thread thread("Thd1",1024*2,1);
Thread thread1("Thd1",1024*2,1);

String DeviceInfo::BoardID="";
String DeviceInfo::Family="k49a";
OLEDScreen<12> oled(Heltec.display);
TracePrinter tracePrinter;
//Test t;
MQTTNetwork mqttNetwork;
LoRaNetwork loRaNetwork;

LoRaCollector loRaCollector(mqttNetwork);
LoRaBeacon loRaBeacon(mqttNetwork);

LoRaGateway loRaGateway(mqttNetwork,loRaNetwork);

void setup() {
 // put your setup code here, to run once:
  /*esp_sleep_wakeup_cause_t cause =  esp_sleep_get_wakeup_cause();
 
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
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,0);
  uint64_t mask = 1|1<<26;
  esp_sleep_enable_ext1_wakeup(mask,ESP_EXT1_WAKEUP_ANY_HIGH);
  */
 //gpio_wakeup_enable(GPIO_NUM_0,GPIO_INTR_POSEDGE)
 //gpio_wakeup_enable(GPIO_NUM_26,GPIO_INTR_POSEDGE)
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  pinMode(22,PULLUP);
  
  #ifdef NDEBUG
    Heltec.begin(false, true , false , true, BAND);
  #else
    Heltec.begin(true, true , true , true, BAND);
  #endif
  //WIFI Kit series V1 not support Vext control
  
  //LoRa.dumpRegisters(Serial);
  PlatformDebug::init(std::move(oled));
  PlatformDebug::printLogo();
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  platform_debug::PlatformDebug::println(" ************ IPS ************ ");
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

  DeviceInfo::BoardID = String(mac_address.substr(mac_address.length()-4,4).c_str());
  platform_debug::PlatformDebug::println("DeviceInfo::BoardID:"+DeviceInfo::BoardID);
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));

  //timeMachine.startup();
  //timeMachine.setEpoch(1614764209+8*60*60);

  //colorSensor.startup();

  loRaNetwork.addOnMessageCallback(callback(&loRaGateway,&LoRaGateway::onMessageLoRaCallback));
  loRaGateway.startup();
 
  loRaNetwork.addOnMessageCallback(callback(&loRaCollector,&LoRaCollector::onMessageLoRaCallback));
  loRaCollector.startup();
  
  //loRaNetwork.addOnMessageCallback(callback(&loRaBeacon,&LoRaBeacon::onMessageLoRaCallback));
  //loRaBeacon.startup();
 ///*
  mqttNetwork.addTopics(loRaGateway.getTopics());
  mqttNetwork.addOnMessageCallback(callback(&loRaGateway,&LoRaGateway::onMessageMqttCallback));
  mqttNetwork.addOnMqttConnectCallback(callback(&loRaGateway,&LoRaGateway::onMqttConnectCallback));
  mqttNetwork.addOnMqttDisonnectCallback(callback(&loRaGateway,&LoRaGateway::onMqttDisconnectCallback));
  // */
 ///*
  mqttNetwork.addTopics(loRaCollector.getTopics());
  mqttNetwork.addOnMessageCallback(callback(&loRaCollector,&LoRaCollector::onMessageMqttCallback));
  mqttNetwork.addOnMqttConnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttConnectCallback));
  mqttNetwork.addOnMqttDisonnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttDisconnectCallback));
  //*/
  /*
  mqttNetwork.addTopics(loRaBeacon.getTopics());
  mqttNetwork.addOnMessageCallback(callback(&loRaBeacon,&LoRaBeacon::onMessageMqttCallback));
  mqttNetwork.addOnMqttConnectCallback(callback(&loRaBeacon,&LoRaBeacon::onMqttConnectCallback));
  mqttNetwork.addOnMqttDisonnectCallback(callback(&loRaBeacon,&LoRaBeacon::onMqttDisconnectCallback));
  */
  
  mqttNetwork.startup();
  loRaNetwork.startup();
  
}

std::array<uint16_t,4> dataRGB;
void loop() {
 
  while (true) {
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(3000));
  }
  
}


  


