#include "TimeMachine.h"
TimeMachine::TimeMachine(DS1307& rtc,rtos::Mutex& mutex)
{
    _rtc=rtc;
    _mutex=&mutex;
    _timestamp=1596851347;
}



void TimeMachine::startup(void *pvParameters)
 {
     do{
     _rtc.begin();
     _rtc.setDateTime(_timestamp);
     _rtc.setHourMode(CLOCK_H24);
     _rtc.startClock();
      digitalWrite(5,HIGH);

      digitalWrite(5,LOW);

  } while(!_rtc.isRunning());
 }