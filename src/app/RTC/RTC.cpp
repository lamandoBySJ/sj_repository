#include "RTC.h"



RTC::RTC(rtos::Mutex& mutex,TwoWire& wire,uint8_t rst):_mtx(mutex),_ds1307(wire),_rst(rst)
{
    
}


bool RTC::init(bool pwrEnable,uint8_t h_mode)
 {
    std::lock_guard<rtos::Mutex> lck(_mtx);
     
    if(pwrEnable){
      pinMode(_rst,OUTPUT);
      digitalWrite(_rst,HIGH);
    }
 
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
       _ds1307.begin();
     if(_ds1307.isRunning()){
      _ds1307.stopClock();
     }
    _ds1307.setHourMode(h_mode);
    _ds1307.startClock();
   return  selftest();    
 }
 
 void RTC::setDateTime(const char* date,const  char* time)
 {
      std::lock_guard<rtos::Mutex> lck(_mtx);
     _ds1307.setDateTime(date,time);
 }


bool RTC::selftest()
{
    int timeout= 30;
    time_t val = 0;
    do
    {   
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(600));
        val =_ds1307.getEpoch();
        if(val != 0 ){
            return true;
        }
    }while(--timeout > 0);
    PlatformDebug::println(String("RTC")+String(__FILE__)+String(":")+String(__LINE__));
    return false;
}


time_t RTC::getEpoch()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    if(_ds1307.isRunning()){
       return _ds1307.getEpoch();
    }
    return 0;
}

void RTC::setEpoch(time_t epoch)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
   _ds1307.setEpoch(epoch);
}


bool RTC::getDateTime(String& datetime,bool duplicate)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    if(_ds1307.isRunning()){
       _ds1307.getDateTime(datetime,duplicate);
        return true;
    }else{
        return false;
    }
}
