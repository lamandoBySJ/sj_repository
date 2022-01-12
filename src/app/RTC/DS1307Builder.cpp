#include "app/RTC/DS1307Builder.h"

void DS1307Builder::power_on()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    if(_rst == -1){
        return;
    }
    pinMode(_rst,OUTPUT);
    digitalWrite(_rst,LOW);
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(2000));
    digitalWrite(_rst,HIGH);
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(500));
}
bool DS1307Builder::init(uint8_t h_mode)
 {
    std::lock_guard<rtos::Mutex> lck(_mtx);
    DS1307::begin();
    DS1307::stopClock();
    DS1307::setHourMode(h_mode);
    DS1307::startClock();
    char n = 6;
    do{
       ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(200));
    }while(!DS1307::isRunning()&& (n-->0));
    return n>0;   
 }
 
bool DS1307Builder::setDateTime(const char* date,const  char* time)
 {
    std::lock_guard<rtos::Mutex> lck(_mtx);
    bool ok = DS1307::setDateTime(date,time);
    if(!ok){
      Logger::error(__PRETTY_FUNCTION__,__LINE__);
    }
    return ok;
 }

time_t DS1307Builder::epoch()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
   // if(DS1307::isRunning()){
        return DS1307::getEpoch();
   // }
   // return 0;
}

void DS1307Builder::setEpoch(time_t epoch)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    DS1307::setEpoch(epoch);
}


