#include <heltec.h>
//#include <ArduinoJson.h>
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
#include "app/NetworkEngine/NetworkEngine.h"
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

using namespace mstd;
using namespace rtos;
using namespace platform_debug;

#define BAND    433E6 
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


NetworkEngine networkEngine;

OLEDScreen<12> oled(Heltec.display);

Test t;
ExceptionCatcher e;
void setup() {
 
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  pinMode(22,PULLUP);
  // put your setup code here, to run once:
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true , false , true , true, BAND);
  
  PlatformDebug::init(std::move(oled));
  PlatformDebug::printLogo();
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  platform_debug::PlatformDebug::println(" ************ IPS ************ ");

  
  e.startup();

  //LoRa.dumpRegisters(Serial);
  //timeMachine.attach(callback(&e,&ExceptionCatcher::PrintTrace));
  //timeMachine.startup();
  
  //timeMachine.setEpoch(1614764209+8*60*60);
  //colorSensor.attach(callback(&e,&ExceptionCatcher::PrintTrace));
  //colorSensor.startup();
  t.attach(callback(&e,&ExceptionCatcher::PrintTrace));
  t.startup();
  networkEngine.addOnMessageCallback(callback(&t,&Test::onMessageCallback));
  networkEngine.attach(callback(&e,&ExceptionCatcher::PrintTrace));
  networkEngine.startup();

 // thread.start(callback(send_thread_mail));
  //thread1.start(callback(send_thread_mail));
  
  while(true){
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));
  }
}


bool n=false;
std::array<uint16_t,4> dataRGB;
void loop() {
 
  static uint32_t cnt = 0;
  static uint32_t i = 0;
  while (true) {
      
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(100));
  }
  
}


  


