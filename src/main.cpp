#include <Arduino.h>
#include <heltec.h>
//#include <ArduinoJson.h>
#include "platform/mbed.h"

using namespace rtos;

#define BAND    433E6 

Mutex  mutex;
char data[]="hello sj~";
void setup() {
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  
  LoRa.dumpRegisters(Serial);

  debug("%d\n",Heltec.display->getStringWidth("a"));
  debug("%d\n",Heltec.display->getStringWidth("b"));
  debug("%d\n",Heltec.display->getStringWidth("c"));
  debug("%d\n",Heltec.display->getStringWidth("d"));
  debug("%d\n",Heltec.display->getStringWidth("e"));
  debug("%d\n",Heltec.display->getStringWidth("f"));
  debug("%d\n",Heltec.display->getStringWidth("g"));
  debug("%d\n",Heltec.display->getStringWidth("h"));
  debug("%d\n",Heltec.display->getStringWidth("i"));
  debug("%d\n",Heltec.display->getStringWidth("j"));
  debug("%d\n",Heltec.display->getStringWidth("k"));
  debug("%d\n",Heltec.display->getStringWidth("l"));
  debug("%d\n",Heltec.display->getStringWidth("m"));
  debug("%d\n",Heltec.display->getStringWidth("n"));
  debug("%d\n",Heltec.display->getStringWidth("o"));
  debug("%d\n",Heltec.display->getStringWidth("p"));
  debug("%d\n",Heltec.display->getStringWidth("q"));
  debug("%d\n",Heltec.display->getStringWidth("r"));
  debug("%d\n",Heltec.display->getStringWidth("s"));
  debug("%d\n",Heltec.display->getStringWidth("t"));
  debug("%d\n",Heltec.display->getStringWidth("u"));
  debug("%d\n",Heltec.display->getStringWidth("v"));
  debug("%d\n",Heltec.display->getStringWidth("w"));
  debug("%d\n",Heltec.display->getStringWidth("x"));
  debug("%d\n",Heltec.display->getStringWidth("y"));
  debug("%d\n",Heltec.display->getStringWidth("z"));
  debug("%d\n",Heltec.display->getStringWidth("A"));
  debug("%d\n",Heltec.display->getStringWidth("B"));
  debug("%d\n",Heltec.display->getStringWidth("C"));
  debug("%d\n",Heltec.display->getStringWidth("D"));
  debug("%d\n",Heltec.display->getStringWidth("E"));
  debug("%d\n",Heltec.display->getStringWidth("F"));
  debug("%d\n",Heltec.display->getStringWidth("G"));
  debug("%d\n",Heltec.display->getStringWidth("H"));
  debug("%d\n",Heltec.display->getStringWidth("I"));
  debug("%d\n",Heltec.display->getStringWidth("J"));
  debug("%d\n",Heltec.display->getStringWidth("K"));
  debug("%d\n",Heltec.display->getStringWidth("L"));
  debug("%d\n",Heltec.display->getStringWidth("M"));
  debug("%d\n",Heltec.display->getStringWidth("N"));
  debug("%d\n",Heltec.display->getStringWidth("O"));
  debug("%d\n",Heltec.display->getStringWidth("P"));
  debug("%d\n",Heltec.display->getStringWidth("Q"));
  debug("%d\n",Heltec.display->getStringWidth("R"));
  debug("%d\n",Heltec.display->getStringWidth("S"));
  debug("%d\n",Heltec.display->getStringWidth("T"));
  debug("%d\n",Heltec.display->getStringWidth("U"));
  debug("%d\n",Heltec.display->getStringWidth("V"));
  debug("%d\n",Heltec.display->getStringWidth("W"));
  debug("%d\n",Heltec.display->getStringWidth("X"));
  debug("%d\n",Heltec.display->getStringWidth("Y"));
  debug("%d\n",Heltec.display->getStringWidth("Z"));
}

void loop() {
  // put your main code here, to run repeatedly:

  //Serial.println("hello");
  //debug_if(true,"debug_if:%d\n",data);
  //debug("%d\n",data);

  /*
   vTaskSuspendAll();       // 开启调度锁      
        printf("任务vTaskLed1正在运行\r\n");   
        if(!xTaskResumeAll())      // 关闭调度锁，如果需要任务切换，此函数返回pdTRUE，否则返回pdFALSE 
        {
            taskYIELD ();
        }    
  */

  //std_mutex.lock();
  //delay(10000);
  ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
}