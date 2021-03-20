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

Test test;

using std::chrono::system_clock;

rtos::Mutex std_mutex;
DS1307 ds1307(Wire1,21,22);
TimeMachine<DS1307> timeMachine(ds1307,std_mutex,13);  
//TimeMachine<RTCBase> timeMachine(&RTC,std_mutex);

BH1749NUC bh1749nuc(Wire1,4,15);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,std_mutex,2);
//ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);

Thread thread("Thd1",1024*2,1);

ExceptionCatcher e;

NetworkEngine networkEngine;

OLEDScreen<12> oled(Heltec.display);

String version="unknow...";
typedef struct {
    float    voltage; /* AD result of measured voltage */
    float    current; /* AD result of measured current */
    uint32_t counter; /* A counter value               */
} mail_t;



typedef struct {
    float    voltage;   /* AD result of measured voltage */
    float    current;   /* AD result of measured current */
    uint32_t counter;   /* A counter value               */
} message_t;

MemoryPool<message_t, 6> mpool;
rtos::Queue<message_t,6> queue;
rtos::Mail<mail_t, 1> mail_box;

void send_thread(void)
{
    uint32_t i = 0;
    while (true) {
        i++; // fake data update
        message_t *message = mpool.alloc();
        message->voltage = (i * 0.1) * 33;
        message->current = (i * 0.1) * 11;
        message->counter = i;
        osStatus  status = queue.put(message);
        ThisThread::sleep_for(100);
    }
  vTaskDelete(NULL);
}

void send_thread_mail(void)
{
    uint32_t i = 0;
    while (true) {
        i++; // fake data update
        mail_t *mail = mail_box.alloc();
        mail->voltage = (i * 0.1) * 33;
        mail->current = (i * 0.1) * 11;
        mail->counter = i;
        mail_box.put(mail);
        ThisThread::sleep_for(1000);
    }
  vTaskDelete(NULL);
} 
void setup() {
 
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true , false , true , true, BAND);
  //Callback<void(const char*)>(&oled,&OLEDScreen<12>::println);
  //PlatformDebug::init(oled);
  PlatformDebug::init(std::move(oled));
  PlatformDebug::printLogo();
  //ThisThread::sleep_for(Kernel::Clock::duration_seconds(2));
  

  //LoRa.dumpRegisters(Serial);
  //timeMachine.attach(callback(&e,&ExceptionCatcher::PrintTrace));
 // timeMachine.startup();
  
  //timeMachine.setEpoch(1614764209+8*60*60);
  //colorSensor.attach(callback(&e,&ExceptionCatcher::PrintTrace));
  //colorSensor.startup();
  //networkEngine.attach(callback(&e,&ExceptionCatcher::PrintTrace));
 // networkEngine.startup();
 
  //thread.start(callback(send_thread_mail));
  platform_debug::PlatformDebug::println("thread.start(callback(send_thread))");
  attachInterrupt(0, []  {
        uint32_t i = 0;
        i++; 
        mail_t *mail = mail_box.alloc();
        mail->voltage = (i * 0.1) * 33;
        mail->current = (i * 0.1) * 11;
        mail->counter = i;
        mail_box.put_from_isr(mail);
    }, FALLING);   
    
}


bool n=false;
std::array<uint16_t,4> dataRGB;
void loop() {
 
  static uint32_t cnt = 0;
  static uint32_t i = 0;

  while (true) {
        osEvent evt = mail_box.get();
        if (evt.status == osEventMail) {
            mail_t *mail = (mail_t *)evt.value.p;
            Serial.printf("\nVoltage: %.2f V\n\r", mail->voltage);
            platform_debug::PlatformDebug::printf("Voltage: %.2f V",mail->voltage);
            Serial.printf("Current: %.2f A\n\r", mail->current);
            platform_debug::PlatformDebug::printf("Current: %.2f A",mail->current);
            Serial.printf("Number of cycles: %lu\n\r", mail->counter);
            platform_debug::PlatformDebug::printf("Number of cycles:%lu",mail->counter);

            mail_box.free(mail);
        } 
        /*
        osEvent evt = queue.get();
        if (evt.status == osEventMessage) {
            message_t *message = (message_t *)evt.value.p;
            Serial.printf("\nVoltage: %.2f V\n\r", message->voltage);
            Serial.printf("Current: %.2f A\n\r", message->current);
            Serial.printf("Number of cycles: %u\n\r", message->counter);
            mpool.free(message);
        }
         Serial.println(String(++cnt,DEC)+"___________________________________"+String(evt.status,DEC));
        */
        
        
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(100));
    }

  
  // put your main code here, to run repeatedly:

  /*
  colorSensor.measurementModeActive();
  colorSensor.getRGB(dataRGB);
  colorSensor.measurementModeInactive();
  debug("%d,%d,%d,%d\n",dataRGB[0],dataRGB[1],dataRGB[2],dataRGB[3]);
  */
  //vTaskDelete(NULL);
}

  


