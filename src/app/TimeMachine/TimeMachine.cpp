#include "TimeMachine.h"

template<typename RTC,typename Mutex>
TimeMachine<RTC,Mutex>::TimeMachine(Mutex& mutex,uint8_t sda,uint8_t scl):_rtc(Wire,sda,scl),_mtx(mutex),_rst(0)
{
    
}
template<typename RTC,typename Mutex>
TimeMachine<RTC,Mutex>::TimeMachine(Mutex& mutex,uint8_t sda,uint8_t scl,uint8_t rst):_rtc(Wire,sda,scl),_mtx(mutex),_rst(rst)
{
    
}

template<typename RTC,typename Mutex>
void TimeMachine<RTC,Mutex>::startup(bool pwrEnable,const char* date,const  char* time)
 {
    std::unique_lock<Mutex> lck(_mtx, std::defer_lock);
    lck.lock(); 
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
    

    if(selftest()){
        digitalWrite(19,HIGH);
    }else{
        digitalWrite(19,LOW);
    }
    
 }
 template<typename RTC,typename Mutex>
 void TimeMachine<RTC,Mutex>::setDateTime(const char* date,const  char* time)
 {
      std::unique_lock<Mutex> lck(_mtx, std::defer_lock);
      lck.lock();
      _rtc.setDateTime(date,time);
 }

template<typename RTC,typename Mutex>
bool TimeMachine<RTC,Mutex>::selftest()
{
    int timeout= 3;
    time_t val = 0;
    do
    {
        ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
   
        val = _rtc.getEpoch();
      
        if(val !=0 ){
            return true;
        }
    }while(--timeout > 0);
    platform_debug::PlatformDebug::println(String("RTC")+String(__FILE__)+String(":")+String(__LINE__));
    return false;
}

template<typename RTC,typename Mutex>
time_t TimeMachine<RTC,Mutex>::getEpoch()
{
    std::unique_lock<Mutex> lck(_mtx, std::defer_lock);
    lck.lock();
 
    if(_rtc.isRunning()){
       return _rtc.getEpoch();
    }
    return 0;
}
template<typename RTC,typename Mutex>
void TimeMachine<RTC,Mutex>::setEpoch(time_t epoch)
{
    std::unique_lock<Mutex> lck(_mtx, std::defer_lock);
    lck.lock();
    _rtc.setEpoch(epoch);
}

template<typename RTC,typename Mutex>
bool TimeMachine<RTC,Mutex>::getDateTime(String& datetime)
{
    std::unique_lock<Mutex> lck(_mtx, std::defer_lock);
    lck.lock();
    if(_rtc.isRunning()){
        datetime =  _rtc.getDateTime(true);
        return true;
    }else{
        return false;
    }
}

template class TimeMachine<DS1307,std::mutex>;
template class TimeMachine<DS1307,rtos::Mutex>;

