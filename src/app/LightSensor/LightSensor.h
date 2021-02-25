#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "Arduino.h"
#include "rtos/rtos.h"

using namespace rtos;
class LightSensor
{
public:
    LightSensor()=delete;
    LightSensor(rtos::Mutex& mutex);
    LightSensor(rtos::Mutex& mutex,uint8_t rst);
    ~LightSensor()=default;
    void startup(void *pvParameters);

private:
    rtos::Mutex& _mutex;
    uint8_t _rst;
};

#endif