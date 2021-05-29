#ifndef __TIME_MACHINE_H
#define __TIME_MACHINE_H

#include "Arduino.h"
#include "DS1307.h"
#include "rtos/rtos.h"
#include "platform_debug.h"
#include <mutex>
using namespace rtos;
extern std::mutex std_mutex;
extern rtos::Mutex stdMutex;
template<typename RTC,typename OSMutex>
class TimeMachine
{
public:
    TimeMachine()=delete;
    TimeMachine(TwoWire& wire,uint8_t sda,uint8_t scl,OSMutex &mutex);
    TimeMachine(TwoWire& wire,uint8_t sda,uint8_t scl,uint8_t rst,OSMutex &mutex);
    ~TimeMachine()=default;
    void init(bool pwrEnable=true,const char* date=nullptr,const  char* time=nullptr);
    time_t getEpoch();
    bool getDateTime(String& datetime,bool duplicate=false);
    void setDateTime(const char* date,const  char* time);
    void setEpoch(time_t epoch);

    
    template<class U = OSMutex, typename std::enable_if_t<std::is_same<U,rtos::Mutex>::value,int> = 0>
    static TimeMachine<RTC,OSMutex>* getTimeMachine(){
        //static TimeMachine<RTC,rtos::Mutex>* rtc = new  TimeMachine<RTC,rtos::Mutex>(Wire,21,22,13,stdMutex);
        static TimeMachine<RTC,rtos::Mutex>* rtc = new  TimeMachine<RTC,rtos::Mutex>
        (platformio_api::I2C_BUS_NUM_RTC() == 0 ? Wire:Wire1
        ,platformio_api::I2C_BUS_SCL_RTC()
        ,platformio_api::I2C_BUS_SDA_RTC()
        ,platformio_api::I2C_BUS_RST_RTC()
        ,stdMutex);
        return rtc;
    }
    
    template<class U = OSMutex, typename std::enable_if_t<std::is_same<U,std::mutex>::value,int> = 0>
    static TimeMachine<RTC,OSMutex>* getTimeMachine(){
       // static TimeMachine<RTC,std::mutex>* rtc = new TimeMachine<RTC,std::mutex>(Wire,21,22,13,std_mutex);
        static TimeMachine<RTC,std::mutex>* rtc = new  TimeMachine<RTC,std::mutex>
        (platformio_api::I2C_BUS_NUM_RTC() == 0 ? Wire:Wire1
        ,platformio_api::I2C_BUS_SCL_RTC()
        ,platformio_api::I2C_BUS_SDA_RTC()
        ,platformio_api::I2C_BUS_RST_RTC()
        ,std_mutex);
        return rtc;
    }
private:
    RTC _rtc;
    OSMutex &_mtx;  
    uint8_t _rst;
    bool selftest();
};



#endif