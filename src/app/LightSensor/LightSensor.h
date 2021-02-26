#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "Arduino.h"
#include "rtos/rtos.h"
#include "Wire.h"
#include <BH1749NUC.h>

using namespace rtos;
class LightSensor
{
public:
    LightSensor()=delete;
    LightSensor(BH1749NUC& wire,rtos::Mutex& mutex);
    LightSensor(BH1749NUC& wire,rtos::Mutex& mutex,uint8_t rst);
    ~LightSensor()=default;
    void startup(void *pvParameters);
    static int32_t thunk_write_reg(void *);
    static int32_t thunk_read_reg(void *);
private:
    BH1749NUC& _bh1749nuc;
    mbed::Callback<void()>  _task_write;
    mbed::Callback<void()>  _task_read;
    void platfrom_write();
    void platfrom_read();
   
    rtos::Mutex& _mutex;
    uint8_t _rst;
};

#endif