#pragma once

#include "app/RTC/RTC.h"
#include "app/RTC/TimeDirector.h"
#include <Wire.h>

class DS1307Builder :public RTCBase,public DS1307
{
public:
    explicit DS1307Builder(rtos::Mutex& mtx,TwoWire& wire,const uint8_t rst=-1):RTCBase(mtx,wire),DS1307(wire),_mtx(mtx),_rst(rst)
    {
   
    }
    explicit DS1307Builder(const DS1307Builder& builder):RTCBase(builder._mtx,builder.I2CBus::_wire),DS1307(builder.I2CBus::_wire)
    ,_mtx(builder._mtx),_rst(builder._rst)
    {
        
    }
    explicit  DS1307Builder(DS1307Builder&& builder):RTCBase(builder._mtx,builder.I2CBus::_wire),DS1307(builder.I2CBus::_wire)
    ,_mtx(builder._mtx),_rst(builder._rst)
    {   
        if(this!=&builder){
            this->~DS1307Builder();
        }
        
    }
    ~DS1307Builder()=default;
  
    void power_on() override;
    bool init(uint8_t h_mode=CLOCK_H24) override;
    time_t epoch() override;
    void setEpoch(time_t epoch) override;
    bool setDateTime(const char* date,const  char* time) override;
private:
    rtos::Mutex& _mtx;
    uint8_t _rst;
};