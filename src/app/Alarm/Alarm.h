#pragma once

#include <heltec.h>

class Alarm
{
public:
      static void open(){
          ins().set(1);
      }
      static void close(){
          ins().set(0);
      }

      static void sound(bool open){
         digitalWrite(32,open?LOW:HIGH);
      }

      static void init(){
        pinMode(32,OUTPUT);
        pinMode(33,INPUT);
        pinMode(23,OUTPUT);
        digitalWrite(23,LOW);
        digitalWrite(32,HIGH);
      }
private:
      Alarm()=default;
      ~Alarm()=default;
      static Alarm& ins(){
        static Alarm alarm;
        return alarm;
      }
      void set(int val){
        digitalWrite(23,val);
      }
};