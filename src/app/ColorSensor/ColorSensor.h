#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include "Arduino.h"
#include "Wire.h"
#include <BH1749NUC.h>
#include <array>
#include <type_traits>
#include "platform_debug.h"

using namespace rtos;
extern std::mutex std_mutex;
extern rtos::Mutex stdMutex;


struct RGB
{
    RGB(){
        R.u16bit = 0;
        G.u16bit = 0;
        B.u16bit = 0;
        IR.u16bit = 0;
    }
    reg_uint16_t R;
    reg_uint16_t G;
    reg_uint16_t B;
    reg_uint16_t IR;
    uint32_t h;
    uint32_t s;
    uint32_t l;
};

class ColorSensor 
{
public:
    ColorSensor()=delete;
    ColorSensor(rtos::Mutex &mtx,TwoWire& wire,uint8_t rst=0);
    ~ColorSensor()=default;
    void power_on();
    bool init();
    bool getRGB(RGB& rgb);
    void measurementModeActive();
    void measurementModeInactive();

    template<typename _Array>
    void loopMeasure(RGB& rgb,const Kernel::Clock::duration_u32& timeInterval,_Array& _rgbTemp)
    {   
        std::unique_lock<Mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        int count =_rgbTemp.size();
        int dim=50/count;
        measurementModeActive();
        for(char i=0;i < count;++i){
            getRGB(_rgbTemp[i]);
            ThisThread::sleep_for(timeInterval);
            if(_measurementHook){
               _measurementHook(i*dim,"measuring","progress");
            }
           // platform_debug::TracePrinter::printTrace(String(_rgbTemp[i].R.u16bit,DEC)+": "+ String(_rgbTemp[i].G.u16bit,DEC)+": "+String(_rgbTemp[i].B.u16bit,DEC)+": "+ String(_rgbTemp[i].IR.u16bit,DEC) ); 
        }
        measurementModeInactive();
       
        std::sort(_rgbTemp.begin(),_rgbTemp.end(),[](const RGB& left,const RGB& right)->bool{
           return left.R.u16bit < right.R.u16bit;
        });
        rgb.R.u16bit=_rgbTemp[count-1].R.u16bit;
        std::sort(_rgbTemp.begin(),_rgbTemp.end(),[](const RGB& left,const RGB& right)->bool{
           return left.G.u16bit < right.G.u16bit;
        });
         rgb.G.u16bit=_rgbTemp[count-1].G.u16bit;
        std::sort(_rgbTemp.begin(),_rgbTemp.end(),[](const RGB& left,const RGB& right)->bool{
           return left.B.u16bit < right.B.u16bit;
        });
        rgb.B.u16bit=_rgbTemp[count-1].B.u16bit;
       /* std::sort(_rgbTemp.begin(),_rgbTemp.end(),[](const RGB& left,const RGB& right)->bool{
           return left.IR.u16bit < right.IR.u16bit;
        });
        rgb.IR.u16bit=_rgbTemp[count-1].IR.u16bit;*/
    }
    using callbackFun=bool(BH1749NUC::*)(reg_uint16_t& value);

    void attachMeasurementHook(std::function<void(int,const String&, const String&)> measHook){
       _measurementHook = measHook;
    }

private:
    rtos::Mutex &_mtx;
    BH1749NUC _bh1749nuc;
    uint8_t _rst;
    std::function<void(int,const String&, const String&)> _measurementHook;
    std::array<callbackFun, 4> ptrFuns;
    bh1749nuc_mode_control2_t _mode_control2;
    
};

#endif