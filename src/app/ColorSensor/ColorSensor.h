#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include "Arduino.h"
#include "rtos/rtos.h"
#include "Wire.h"
#include <BH1749NUC.h>
#include "BH1749NUC_REG/bh1749nuc_reg.h"
#include <ColorSensorBase.h>
#include <cxxsupport/mstd_type_traits.h>
#include "platform_debug.h"
#include <mutex>
using namespace platform_debug;
using namespace rtos;
struct RGB
{
    rgb1bit16_t R;
    rgb1bit16_t G;
    rgb1bit16_t B;
    rgb1bit16_t IR;
};
template<typename T>
class ColorSensor 
{
public:
    ColorSensor()=delete;
    ColorSensor(T&,std::mutex& mutex);
    ColorSensor(T&,std::mutex& mutex,uint8_t rst);
    ~ColorSensor()=default;
    void startup(bool pwrEnable=true);
    bool getRGB(RGB& rgb);
    void measurementModeActive();
    void measurementModeInactive();
private:
    T& _colorSensor;
    std::mutex& _mtx;
    uint8_t _rst;
    using callbackFun=bool(T::*)(rgb1bit16_t& value);
    std::array<callbackFun, 4> ptrFuns;
    bh1749nuc_mode_control2_t _mode_control2;
};

#endif