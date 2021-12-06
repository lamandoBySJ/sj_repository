#pragma once
#include "Arduino.h"
#include <atomic>
#include "Wire.h"
#include "platformio_api.h"

class I2CBus
{
public:
    I2CBus(rtos::Mutex& mtx,TwoWire& wire):_mtx(mtx),_wire(wire){}
    ~I2CBus()=default;

    bool begin(int sda,int scl,uint32_t frequency=100000)
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        bool ok = (I2CBus::sda()==sda && I2CBus::scl()==scl);
        if(!ok){
             ok = _wire.begin(sda,scl,frequency);
            I2CBus::sda()      = sda;
            I2CBus::scl()      = scl;
            I2CBus::frequency()=frequency;
        }
        return ok;
    }

   static std::atomic<int>& sda(){static std::atomic<int> sda(-1); return sda;} 
   static std::atomic<int>& scl(){static std::atomic<int> scl(-1); return scl;} 
   static std::atomic<uint32_t>& frequency(){static std::atomic<uint32_t> frequency(-1); return frequency;} 
    rtos::Mutex& _mtx;
    TwoWire& _wire;
};