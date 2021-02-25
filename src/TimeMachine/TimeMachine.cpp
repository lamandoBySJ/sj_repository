
#include "TimeMachine.h"

//extern  rtos::Mutex _mutex;

TimeMachine::TimeMachine(DS1307& rtc,rtos::Mutex& mutex):_rtc(rtc),_mutex(mutex)
{
    _rst = 0;
}
TimeMachine::TimeMachine(DS1307& rtc,rtos::Mutex& mutex,uint8_t rst):_rtc(rtc),_mutex(mutex)
{
    _rst = rst;
}
 
void TimeMachine::startup(void *pvParameters)
 {
    if(pvParameters != NULL){
      digitalWrite(_rst,HIGH);
      ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
    }

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
void TimeMachine::setEpoch(time_t epoch)
{
     _mutex.lock();
     _rtc.setEpoch(epoch);
    _mutex.unlock();
}

String TimeMachine::getDateTime()
 {
     String datetime="";
     _mutex.lock();
     if(_rtc.isRunning()){
        datetime = _rtc.getDateTime(true);
     }
    _mutex.unlock();
    return datetime;
 }