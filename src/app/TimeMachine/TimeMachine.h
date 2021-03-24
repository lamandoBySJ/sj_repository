#ifndef TIME_MACHINE_H
#define TIME_MACHINE_H

#include "Arduino.h"
#include "DS1307.h"
#include "rtos/rtos.h"
#include "RTCBase.h"
#include "DelegateClass.h"
#include "platform_debug.h"
using namespace rtos;

template<typename RTC>
class TimeMachine
{
public:
    TimeMachine()=delete;
    TimeMachine(RTC& rtc,rtos::Mutex& mutex);
    TimeMachine(RTC& rtc,rtos::Mutex& mutex,uint8_t rst);
    ~TimeMachine()=default;
    void startup(bool pwrEnable=true);
    time_t getEpoch();
    String getDateTime();
    void setEpoch(time_t epoch);
private:
    RTC& _rtc;
    rtos::Mutex& _mutex;
    uint8_t _rst;
    bool selftest();
};



#endif