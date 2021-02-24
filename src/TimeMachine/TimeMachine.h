#ifndef TIME_MACHINE_H
#define TIME_MACHINE_H

#include "Arduino.h"
#include "DS1307.h"
#include "rtos/rtos.h"

using namespace rtos;
class TimeMachine
{
public:
    TimeMachine()=delete;
    TimeMachine(DS1307& rtc,rtos::Mutex& mutex);
    TimeMachine(DS1307& rtc,rtos::Mutex& mutex,uint8_t rst);
    ~TimeMachine()=default;
    void startup(void *pvParameters);
    time_t getEpoch();
    String getDateTime();
private:
    DS1307& _rtc;
    rtos::Mutex& _mutex;
    uint8_t _rst;
};

#endif