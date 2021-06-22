#include "RTC.h"



RTC::RTC(rtos::Mutex& mutex,TwoWire& wire,uint8_t rst):_mtx(mutex),_ds1307(wire),_rst(rst)
{
    
}
void RTC::power_on()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    pinMode(_rst,OUTPUT);
    digitalWrite(_rst,LOW);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    digitalWrite(_rst,HIGH);
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));
}
bool RTC::init(uint8_t h_mode)
 {
    std::lock_guard<rtos::Mutex> lck(_mtx);
    _ds1307.begin();
     if(_ds1307.isRunning()){
      _ds1307.stopClock();
     }
    _ds1307.setHourMode(h_mode);
    _ds1307.startClock();

    int timeout= 6;
    time_t val = 0;
    do
    {   
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(600));
        val =_ds1307.getEpoch();
        if(val != 0 ){
            return true;
        }
    }while(--timeout > 0);
    return false;   
 }
 
bool RTC::setDateTime(const char* date,const  char* time)
 {
    std::lock_guard<rtos::Mutex> lck(_mtx);
    bool ok = _ds1307.setDateTime(date,time);
    if(!ok){
      Logger::error(__PRETTY_FUNCTION__,__LINE__);
    }
    return ok;
 }

time_t RTC::getEpoch()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    if(_ds1307.isRunning()){
       return _ds1307.getEpoch();
    }else{
        Logger::error(__PRETTY_FUNCTION__,__LINE__);
    }
    return 0;
}

void RTC::setEpoch(time_t epoch)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
   _ds1307.setEpoch(epoch);
}


bool RTC::getDateTime(String& datetime)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    if(_ds1307.isRunning()){
       _ds1307.getDateTime(datetime);
        return true;
    }else{
        return false;
    }
}