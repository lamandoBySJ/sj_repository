#ifndef __TIME_MACHINE_H
#define __TIME_MACHINE_H

#include "Arduino.h"
#include "DS1307.h"
#include "platform_debug.h"

using namespace rtos;


class RTC
{
public:
    explicit RTC()=delete;
    explicit RTC(rtos::Mutex& mutex,TwoWire& wire=Wire1,uint8_t rst=0);
    ~RTC()=default;
    bool init(bool pwrEnable=false,uint8_t h_mode=CLOCK_H24);
    time_t getEpoch();
    bool getDateTime(String& datetime,bool duplicate=false);
    void setDateTime(const char* date,const  char* time);
    void setEpoch(time_t epoch);
private:
    rtos::Mutex& _mtx;
    DS1307 _ds1307;
    uint8_t _rst;
    bool selftest();
};



#endif