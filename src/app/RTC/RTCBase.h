#pragma once 
#include "Arduino.h"
#include "DS1307.h"
#include "platformio_api.h"
#include "app/I2CBus/I2CBus.h"
#include "Wire.h"


class RTCBase : public I2CBus
{
public:
    RTCBase()=delete;
    RTCBase(rtos::Mutex& mtx,TwoWire& wire):I2CBus(mtx,wire){}
    ~RTCBase()=default;
    virtual void power_on() = 0;
    virtual bool init(uint8_t h_mode=CLOCK_H24)= 0;
	virtual  bool setDateTime(const char* date,const  char* time) = 0;
	virtual time_t epoch() = 0;
    virtual void setEpoch(time_t epoch) = 0;

    static	RTCBase** getInstance(){
		static RTCBase* _base;
		return &_base;
	}
};

