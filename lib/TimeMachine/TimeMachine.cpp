#include "TimeMachine.h"

TimeMachine::TimeMachine(DS1307& rtc,rtos::Mutex& mutex):_rtc(rtc),_mutex(mutex)
{
 
}

void TimeMachine::startup(void *pvParameters)
 {
     _mutex.lock();
     _rtc.begin();
     _rtc.setEpoch(1610000000);
     _rtc.setHourMode(CLOCK_H24);
     _rtc.startClock();
    _mutex.unlock();
 }
 time_t TimeMachine::getEpoch()
 {
     time_t epoch=0;
     _mutex.lock();
     if(_rtc.isRunning()){
       epoch = _rtc.getEpoch();
     }
    _mutex.unlock();
    return epoch;
 }

String TimeMachine::getDateTime()
 {
     String datetime="";
     _mutex.lock();
     if(_rtc.isRunning()){
        datetime = _rtc.getDateTime();
     }
    _mutex.unlock();
    return datetime;
 }