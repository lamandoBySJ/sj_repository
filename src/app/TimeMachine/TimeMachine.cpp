#include "TimeMachine.h"

template<typename RTC,typename OSMutex>
TimeMachine<RTC,OSMutex>::TimeMachine(TwoWire& wire,uint8_t sda,uint8_t scl,OSMutex &mutex):_rtc(wire,sda,scl),_mtx(mutex),_rst(0)
{
    
}
template<typename RTC,typename OSMutex>
TimeMachine<RTC,OSMutex>::TimeMachine(TwoWire& wire,uint8_t sda,uint8_t scl,uint8_t rst,OSMutex &mutex):_rtc(wire,sda,scl),_mtx(mutex),_rst(rst)
{
    
}

template<typename RTC,typename OSMutex>
void TimeMachine<RTC,OSMutex>::init(bool pwrEnable,const char* date,const  char* time)
 {
    std::lock_guard<OSMutex> lck(_mtx);
     
    if(pwrEnable){
      pinMode(_rst,OUTPUT);
      digitalWrite(_rst,HIGH);
    }
 
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
      _rtc.begin();
     if(_rtc.isRunning()){
       _rtc.stopClock();
     }

     if(date!=nullptr&&time!=nullptr){
         _rtc.setDateTime(date,time);
     }
     _rtc.setHourMode(CLOCK_H24);
     _rtc.startClock();
    

    selftest();
       
 }
 template<typename RTC,typename OSMutex>
 void TimeMachine<RTC,OSMutex>::setDateTime(const char* date,const  char* time)
 {
      std::lock_guard<OSMutex> lck(_mtx);
      _rtc.setDateTime(date,time);
 }

template<typename RTC,typename OSMutex>
bool TimeMachine<RTC,OSMutex>::selftest()
{
    int timeout= 30;
    time_t val = 0;
    do
    {   
        LEDIndicator::getLEDIndicator().io_state_rtc(true);
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(200));
        val = _rtc.getEpoch();
        if(val == 0 ){
            LEDIndicator::getLEDIndicator().io_state_rtc(false);
            ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(200));
        }else{
            return true;
        }
    }while(--timeout > 0);
    PlatformDebug::println(String("RTC")+String(__FILE__)+String(":")+String(__LINE__));
    return false;
}

template<typename RTC,typename OSMutex>
time_t TimeMachine<RTC,OSMutex>::getEpoch()
{
    std::lock_guard<OSMutex> lck(_mtx);
    if(_rtc.isRunning()){
       return _rtc.getEpoch();
    }
    return 0;
}
template<typename RTC,typename OSMutex>
void TimeMachine<RTC,OSMutex>::setEpoch(time_t epoch)
{
    std::lock_guard<OSMutex> lck(_mtx);
    _rtc.setEpoch(epoch);
}

template<typename RTC,typename OSMutex>
bool TimeMachine<RTC,OSMutex>::getDateTime(String& datetime,bool duplicate)
{

    std::lock_guard<OSMutex> lck(_mtx);
    if(_rtc.isRunning()){
        _rtc.getDateTime(datetime,duplicate);
        return true;
    }else{
        return false;
    }
}

template class TimeMachine<DS1307,std::mutex>;
template class TimeMachine<DS1307,rtos::Mutex>;

