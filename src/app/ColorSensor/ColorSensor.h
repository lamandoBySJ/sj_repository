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
#include <array>

using namespace platform_debug;
using namespace rtos;


template<typename T>
class ColorSensor 
{
public:
    ColorSensor()=default;
    ColorSensor(TwoWire& wire,uint8_t  sda,uint8_t scl,std::mutex& mutex);
    ColorSensor(TwoWire& wire,uint8_t  sda,uint8_t scl,std::mutex& mutex,uint8_t rst);
    ~ColorSensor()=default;
    void init(bool pwrEnable=true);
    bool getRGB(RGB& rgb);
    void measurementModeActive();
    void measurementModeInactive();

    template<typename _Rep, typename _Period,typename _Array>
    void loopMeasure(RGB& rgb,const std::chrono::duration<_Rep,_Period> &timeInterval,_Array& _rgbTemp)
    {   
        
        int count =_rgbTemp.size();
        measurementModeActive();
        for(char i=0;i < count;++i){
            getRGB(_rgbTemp[i]);
            std::this_thread::sleep_for(timeInterval);
        }
        measurementModeInactive();
        /*
        std::sort(_rgbTemp.begin(),_rgbTemp.end(),[](const RGB& left,const RGB& right)->bool{
           return left.R.u16bit < right.R.u16bit;
        });
        std::sort(_rgbTemp.begin(),_rgbTemp.end(),[](const RGB& left,const RGB& right)->bool{
           return left.G.u16bit < right.G.u16bit;
        });
        std::sort(_rgbTemp.begin(),_rgbTemp.end(),[](const RGB& left,const RGB& right)->bool{
           return left.B.u16bit < right.B.u16bit;
        });
        std::sort(_rgbTemp.begin(),_rgbTemp.end(),[](const RGB& left,const RGB& right)->bool{
           return left.IR.u16bit < right.IR.u16bit;
        });

        rgb.R.u16bit=_rgbTemp[count-1].R.u16bit;
        rgb.G.u16bit=_rgbTemp[count-1].G.u16bit;
        rgb.B.u16bit=_rgbTemp[count-1].B.u16bit;
        rgb.IR.u16bit=_rgbTemp[count-1].IR.u16bit;*/
    }
    using callbackFun=bool(T::*)(reg_uint16_t& value);
private:
    T _als;
    std::mutex& _mtx;
    uint8_t _rst;
    std::array<callbackFun, 4> ptrFuns;
    bh1749nuc_mode_control2_t _mode_control2;
};

#endif