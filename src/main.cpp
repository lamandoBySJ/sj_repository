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

extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}

#include <app/AsyncMqttClient/AsyncMqttClient.h>

using namespace mstd;
using namespace rtos;

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


void WiFiEvent(system_event_id_t event) {
    /*
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        networkEngine.setMqttReconnectTimer(true);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection"); 
        // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        if(thunk_event != SYSTEM_EVENT_STA_DISCONNECTED){
            networkEngine.setMqttReconnectTimer(false);
            networkEngine.setWifiReconnectTimer(true);
        }
        break;
    default:break;
    }
    thunk_event = event;*/
}
void setup() {
  
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true , false , true , true, BAND);
  //LoRa.dumpRegisters(Serial);
  timeMachine.attach(delegate(&e,&ExceptionCatcher::PrintTrace));
  timeMachine.startup();
  
  //timeMachine.setEpoch(1614764209+8*60*60);
  colorSensor.attach(delegate(&e,&ExceptionCatcher::PrintTrace));
  colorSensor.startup();

  //ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));

  networkEngine.startup();
 //Callback<void(String,String,int)> call(&a,&A::Fun6);
 //call.call(String("ABC"),String("ABC"),1);

  //Callback<void(String&&,String&&,int&&)>  call();
  //MyTest t;
  //  A a;
 // t.Fun(Callback<void(String&&,String&&,int&&)>(&a,&A::Fun6));

}

bool n=false;
std::array<uint16_t,4> dataRGB;
void loop() {
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
  // put your main code here, to run repeatedly:
    n=networkEngine.publish("test/lol","xxx");
   debug("__cplusplus:%ld,%d\n",__cplusplus,n);
   
  /*
  colorSensor.measurementModeActive();
  colorSensor.getRGB(dataRGB);
  colorSensor.measurementModeInactive();
  debug("%d,%d,%d,%d\n",dataRGB[0],dataRGB[1],dataRGB[2],dataRGB[3]);
  */
  //vTaskDelete(NULL);
}

  


