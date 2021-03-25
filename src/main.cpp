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
#include "LoRaGatewayMaster.h"
#include "LoRaNetwork.h"

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


NetworkEngine networkEngine;
LoRaGatewayMaster master;
OLEDScreen<12> oled(Heltec.display);

//Test t;
//ExceptionCatcher e;
String DeviceInfo::BoardID="";
TracePrinter tracePrinter;
LoRaNetwork loRaNetwork;
void setup() {
 
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  pinMode(22,PULLUP);

  // put your setup code here, to run once:
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true , true , true , true, BAND);
  
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
  //LoRa.dumpRegisters(Serial);
  //timeMachine.attach(callback(&e,&ExceptionCatcher::PrintTrace));
  //timeMachine.startup();
  
  //timeMachine.setEpoch(1614764209+8*60*60);
  //colorSensor.attach(callback(&e,&ExceptionCatcher::PrintTrace));
  //colorSensor.startup();
  //t.startup();
  
  master.startup();

  loRaNetwork.addOnMessageCallback(callback(&master,&LoRaGatewayMaster::onMessageLoRaCallback));
  loRaNetwork.startup();

  networkEngine.addTopic("Server/Request/"+DeviceInfo::BoardID);
  networkEngine.addOnMessageCallback(callback(&master,&LoRaGatewayMaster::onMessageMqttCallback));
  networkEngine.addOnMqttConnectCallback(callback(&master,&LoRaGatewayMaster::onMqttConnectCallback));
  networkEngine.addOnMqttDisonnectCallback(callback(&master,&LoRaGatewayMaster::onMqttDisconnectCallback));
  networkEngine.startup();

 // thread.start(callback(send_thread_mail));
  //thread1.start(callback(send_thread_mail));
  
}


bool n=false;
std::array<uint16_t,4> dataRGB;
void loop() {
 
  while (true) {
      
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));
  }
  
}


  


