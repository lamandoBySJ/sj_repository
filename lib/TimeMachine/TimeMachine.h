#ifndef TIME_MACHINE_H
#define TIME_MACHINE_H

#include "DS1307.h"
#include <rtos/rtos.h>

class TimeMachine
{
public:
    TimeMachine(DS1307& rtc,rtos::Mutex& mutex);
    ~TimeMachine()=default;
   void startup(void *pvParameters);
   void startup(time_t timestamp);
private:
    DS1307& _rtc;
    rtos::Mutex* _mutex;
    time_t _timestamp;
};

#endif