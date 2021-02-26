#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include "Arduino.h"
#include "rtos/rtos.h"
#include "Wire.h"
#include <BH1749NUC.h>
#include <ColorSensorBase.h>

using namespace rtos;

template<typename T>
class ColorSensor 
{
public:
    ColorSensor()=delete;
    ColorSensor(T& als,rtos::Mutex& mutex);
    ColorSensor(T& als,rtos::Mutex& mutex,uint8_t rst);
    ~ColorSensor()=default;
    void startup(void *pvParameters);

private:
    T& _als;
    rtos::Mutex& _mutex;
    uint8_t _rst;
};

template<>
class ColorSensor<ColorSensorBase>
{
public:
    ColorSensor()=delete;
    ColorSensor(ColorSensorBase* csb,rtos::Mutex& mutex):_csb(csb),_als(*_csb),_mutex(mutex)
    {
        
    }
    ColorSensor(ColorSensorBase* csb,rtos::Mutex& mutex,uint8_t rst):_csb(csb),_als(*_csb),_mutex(mutex)
    {

    }
    ~ColorSensor()=default;
    void startup(void *pvParameters)
    {

    }

private:

    ColorSensorBase* _csb;
    ColorSensorBase& _als;
    rtos::Mutex& _mutex;
    uint8_t _rst;
    void platfrom_write()
    {

    }
    void platfrom_read()
    {

    }

};



#endif