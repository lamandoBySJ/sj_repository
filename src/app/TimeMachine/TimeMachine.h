#ifndef __TIME_MACHINE_H
#define __TIME_MACHINE_H

#include "Arduino.h"
#include "DS1307.h"
#include "rtos/rtos.h"
#include "RTCBase.h"
#include "platform_debug.h"
#include <mutex>
using namespace rtos;

template<typename RTC,typename Mutex>
class TimeMachine
{
public:
    TimeMachine()=delete;
    TimeMachine(Mutex& mutex,uint8_t sda,uint8_t scl);
    TimeMachine(Mutex& mutex,uint8_t sda,uint8_t scl,uint8_t rst);
    ~TimeMachine()=default;
    void startup(bool pwrEnable=true,const char* date=nullptr,const  char* timee=nullptr);
    time_t getEpoch();
    bool getDateTime(String&);
    void setDateTime(const char* date,const  char* time);
    void setEpoch(time_t epoch);
private:
    RTC _rtc;
    Mutex& _mtx;  
    uint8_t _rst;
    bool selftest();
};



#endif