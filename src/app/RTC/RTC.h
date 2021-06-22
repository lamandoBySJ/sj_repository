#ifndef __TIME_MACHINE_H
#define __TIME_MACHINE_H

#include "Arduino.h"
#include "DS1307.h"
#include "platform_debug.h"
#include "Logger.h"
using namespace rtos;


class RTC
{
public:
    explicit RTC()=delete;
    explicit RTC(rtos::Mutex& mutex,TwoWire& wire=Wire1,uint8_t rst=0);
    ~RTC()=default;
    void power_on();
    
    bool init(uint8_t h_mode=CLOCK_H24);
    time_t getEpoch();
    bool getDateTime(String& datetime);
    bool setDateTime(const char* date,const  char* time);
    void setEpoch(time_t epoch);
    static time_t& TimeSyncEpoch(){
        static time_t epoch=28800;
        return epoch;
    }
    static uint32_t& TimeSyncCountup(){
        static uint32_t count=0;
        return count;
    }
private:
    rtos::Mutex& _mtx;
    DS1307 _ds1307;
    uint8_t _rst;
};



#endif