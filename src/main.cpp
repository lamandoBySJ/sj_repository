#include <Arduino.h>
#include <heltec.h>
//#include <ArduinoJson.h>
#include "platform/mbed.h"
#include <rtos/Thread.h>
#include <rtos/rtos.h>
#include "TimeMachine.h"

using namespace rtos;

#define BAND    433E6 
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

TimeMachine RTC;
//rtos::Mutex  std_mutex;
Thread thread;
char data[]="hello sj~";

xTaskHandle handleTaskDebug=NULL;
void TaskDebug( void *pvParameters );
void TaskTest( void *pvParameters );

class Test{
public:
    Test(){};
    ~Test(){};
    void run(){
      for(;;){
        debug("Test Callback ...\n");
        ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
      }
    }
 
    void startup(){
      thread.start(callback(this,&Test::run));
    }
};

Test test;
void setup() {
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  
  //LoRa.dumpRegisters(Serial);
  
  RTC.startup(NULL);

  /*
  xTaskCreatePinnedToCore(
    TaskDebug
    ,  "TaskDebug"
    ,  8*1024  
    ,  NULL
    ,  1  
    ,  &handleTaskDebug
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskTest
    ,  "TaskTest"
    ,  8*1024  
    ,  NULL
    ,  5  
    ,  &handleTaskDebug
    ,  ARDUINO_RUNNING_CORE);
  
  attachInterrupt(0, []  {
    vTaskSuspend(handleTaskDebug);
  }, FALLING);   
  */
  debug("__cplusplus:%d , RTC:%d,%s\n", __cplusplus,RTC.getEpoch(),RTC.getDateTime());
 
  test.startup();
  
}
int cnt=0;
void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("hello");
  //debug_if(true,"debug_if:%d\n",data);
  //delay(10000);
  ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  if(++cnt==10){
    cnt=0;
    vTaskResume(handleTaskDebug);
  }
  debug("%s:%d\n",data,cnt);
}

void TaskDebug( void *pvParameters )
{
  int x = 0;
  for(;;){
      //std_mutex.lock();
      debug("task debug ...%d\n",++x);
      //std_mutex.unlock();
      ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  }
}
void TaskTest( void *pvParameters )
{
  int x = 0;
  for(;;){
      //std_mutex.lock();
      debug("task test ......%d\n",++x);
      //std_mutex.unlock();
      ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  }
}
