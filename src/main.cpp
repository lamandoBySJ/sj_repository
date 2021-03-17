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
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}

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

Test test;

using std::chrono::system_clock;

rtos::Mutex std_mutex;
DS1307 ds1307(Wire,21,22);
TimeMachine<DS1307> timeMachine(ds1307,std_mutex,13);  
//TimeMachine<RTCBase> timeMachine(&RTC,std_mutex);

rtos::Mutex mutex;
BH1749NUC bh1749nuc(Wire1,4,15);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,mutex,2);
//ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);

Thread thread1("Thd1",1024*2,1);

ExceptionCatcher e;
String ExceptionCatcher::exceptionType="";

NetworkEngine networkEngine;

OLEDScreen<12> oled(Heltec.display);

void setup() {
 
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true , false , true , true, BAND);
  //Callback<void(const char*)>(&oled,&OLEDScreen<12>::println);
  PlatformDebug::init(std::move(oled));
  PlatformDebug::printLogo();
  /*
  while(1){
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(2));
    PlatformDebug::print("abcdefgzgdfklgfdklgkl");
  }*/
  
  //LoRa.dumpRegisters(Serial);
  timeMachine.attach(delegate(&e,&ExceptionCatcher::PrintTrace));
  timeMachine.startup();
  
  //timeMachine.setEpoch(1614764209+8*60*60);
  colorSensor.attach(delegate(&e,&ExceptionCatcher::PrintTrace));
  colorSensor.startup();
  
  networkEngine.startup();

}

bool n=false;
std::array<uint16_t,4> dataRGB;
void loop() {
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
  // put your main code here, to run repeatedly:

  /*
  colorSensor.measurementModeActive();
  colorSensor.getRGB(dataRGB);
  colorSensor.measurementModeInactive();
  debug("%d,%d,%d,%d\n",dataRGB[0],dataRGB[1],dataRGB[2],dataRGB[3]);
  */
  //vTaskDelete(NULL);
}

  


