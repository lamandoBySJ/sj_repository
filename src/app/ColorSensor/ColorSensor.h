#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include "Arduino.h"
#include "rtos/rtos.h"
#include "Wire.h"
#include <BH1749NUC.h>
#include <ColorSensorBase.h>
#include <cxxsupport/mstd_type_traits.h>
#include "DelegateClass.hpp"
using namespace rtos;

template<typename T>
class ColorSensor 
{
public:
    ColorSensor()=delete;
    ColorSensor(T&,rtos::Mutex& mutex);
    ColorSensor(T&,rtos::Mutex& mutex,uint8_t rst);
    ~ColorSensor()=default;
    void startup(bool pwrEnable=true);
    bool getRGB(std::array<uint16_t,4>& data);
    using callbackFun=bool(T::*)(uint16_t& value);
    void attach(DelegateClass<void(ExceptionType,String)> func);
    void measurementModeActive();
    void measurementModeInactive();
private:
    T& _colorSensor;
    rtos::Mutex& _mutex;
    uint8_t _rst;
    std::array<callbackFun, 4> ptrFuns;
    std::array<uint16_t,4> _data;
    DelegateClass<void(ExceptionType,String)> _delegate;
    bh1749nuc_mode_control2_t _mode_control2;
};

#endif