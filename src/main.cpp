#include <Arduino.h>
#include <heltec.h>
//#include <ArduinoJson.h>
#include "platform/mbed.h"

using namespace rtos;

#define BAND    433E6 

rtos::Mutex  mutex;
char data[]="hello sj~";
void setup() {
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  
  LoRa.dumpRegisters(Serial);


}
/*
   vTaskSuspendAll();       // 开启调度锁      
        printf("任务vTaskLed1正在运行\r\n");   
        if(!xTaskResumeAll())      // 关闭调度锁，如果需要任务切换，此函数返回pdTRUE，否则返回pdFALSE 
        {
            taskYIELD ();
        }    
*/
void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("hello");
  debug_if(true,"debug_if:%d\n",data);
  debug("%d\n",data);
  //std_mutex.lock();
  //delay(10000);
  ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
}