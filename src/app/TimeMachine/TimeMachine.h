#ifndef TIME_MACHINE_H
#define TIME_MACHINE_H

#include "Arduino.h"
#include "DS1307.h"
#include "rtos/rtos.h"
#include "RTCBase.h"
using namespace rtos;

template<typename RTC>
class TimeMachine
{
public:
    TimeMachine()=delete;
    TimeMachine(RTC& rtc,rtos::Mutex& mutex);
    TimeMachine(RTC& rtc,rtos::Mutex& mutex,uint8_t rst);
    ~TimeMachine()=default;
    void startup(void *pvParameters);
    time_t getEpoch();
    String getDateTime();
    void setEpoch(time_t epoch);
private:
    RTC& _rtc;
    rtos::Mutex& _mutex;
    uint8_t _rst;
};


template<>
class TimeMachine<RTCBase>
{
public:
    TimeMachine()=delete;
 
    TimeMachine(RTCBase* rtc,rtos::Mutex& mutex):_thunk_rtc(rtc),_rtc(*_thunk_rtc),_mutex(mutex)
    {
        //_rtc=(*_thunk_rtc);
        _rst = 0;
    }
    TimeMachine(RTCBase* rtc,rtos::Mutex& mutex,uint8_t rst):_thunk_rtc(rtc),_rtc(*_thunk_rtc),_mutex(mutex)
    {
        //_rtc=(*_thunk_rtc);
        _rst = rst;
    }
    ~TimeMachine()=default;
    void startup(void *pvParameters)
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
    time_t getEpoch()
    {
        time_t epoch=0;
        _mutex.lock();
        if(_rtc.isRunning()){
            epoch = _rtc.getEpoch();
        }
        _mutex.unlock();
        return epoch;
    }
    String getDateTime()
    {
         _mutex.lock();
        String datetime="";
        if(_rtc.isRunning()){
            datetime = _rtc.getDateTime(true);
        }
        _mutex.unlock();
        return datetime;
    }
    void setEpoch(time_t epoch)
    {
         _mutex.lock();
        _rtc.setEpoch(epoch);
        _mutex.unlock();
    }
private:
    RTCBase* _thunk_rtc;
    RTCBase& _rtc;
    rtos::Mutex& _mutex;
    uint8_t _rst;
};

#endif