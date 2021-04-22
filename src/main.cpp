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
#include <cxxsupport/mstd_new.h>
#include <new>
#include <mutex>
#include <thread>
#include <functional>
#include <Test.h>

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

std::mutex mtx;           // mutex for critical section
rtos::Mutex std_mutex;
//DS1307 ds1307(Wire1,21,22); //stlb
DS1307 ds1307(Wire1,32,33); //ips
TimeMachine<DS1307> timeMachine(ds1307,mtx,13);  
//TimeMachine<RTCBase> timeMachine(&RTC,std_mutex);

BH1749NUC bh1749nuc(Wire1,4,15);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,mtx,2);
//ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);

String DeviceInfo::BoardID="";
String DeviceInfo::Family="k49a";
OLEDScreen<12> oled(Heltec.display);
TracePrinter tracePrinter;
MQTTNetwork mqttNetwork;
//Test t;
using namespace thread_test;

#define OLEDSCREEN 1
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
    #ifdef OLEDSCREEN
    #undef OLEDSCREEN
    #endif
    Heltec.begin(false, true , false , true, BAND);
  #else
    #ifdef OLEDSCREEN
      Heltec.begin(true, true , true , true, BAND);
      PlatformDebug::init(Serial,OLEDScreen<12>(Heltec.display));
      PlatformDebug::printLogo();
    #else
      Heltec.begin(false, true , true , true, BAND);
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


  timeMachine.startup();
  //timeMachine.setEpoch(1614764209+8*60*60);

  //colorSensor.startup();
  //mqttNetwork.addTopics(loRaCollector.getTopics());
  //mqttNetwork.addOnMessageCallback(callback(&loRaCollector,&LoRaCollector::onMessageMqttCallback));
  //mqttNetwork.addOnMqttConnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttConnectCallback));
  //mqttNetwork.addOnMqttDisonnectCallback(callback(&loRaCollector,&LoRaCollector::onMqttDisconnectCallback));
  
  mqttNetwork.startup();

  //std::thread threads[2];
  //threads[0]= std::thread(print_thread_id_test, 1);
  //threads[1]= std::thread(print_thread_id_test2, 2);

 // test_unique_lock_cd();
 
 // while(1){  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));};
 platform_debug::PlatformDebug::println(">>>>>>>>>>>>>>>>> Setup over >>>>>>>>>>>>>>>>>>");
}

std::array<uint16_t,4> dataRGB;
void loop() {
  while (true) {
      platform_debug::PlatformDebug::println("Time:" + timeMachine.getDateTime());
      //ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(3000));
      std::this_thread::sleep_for(chrono::seconds(1));
  }
}


  


