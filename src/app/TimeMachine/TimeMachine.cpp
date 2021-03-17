#include "TimeMachine.h"

//extern  rtos::Mutex _mutex;
template<typename RTC>
TimeMachine<RTC>::TimeMachine(RTC& rtc,rtos::Mutex& mutex):_rtc(rtc),_mutex(mutex)
{
    _rst = 0;
}
template<typename RTC>
TimeMachine<RTC>::TimeMachine(RTC& rtc,rtos::Mutex& mutex,uint8_t rst):_rtc(rtc),_mutex(mutex)
{
    _rst = rst;
}

template<typename RTC>
void TimeMachine<RTC>::startup(bool pwrEnable)
 {
     
    if(pwrEnable){
      pinMode(_rst,OUTPUT);
      digitalWrite(_rst,HIGH);
    }
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
     _mutex.lock();
      _rtc.begin();
     if(_rtc.isRunning()){
       _rtc.stopClock();
     }
      _rtc.setDateTime(__DATE__,__TIME__);
     //_rtc.setEpoch(1610000000);
     _rtc.setHourMode(CLOCK_H24);
     _rtc.startClock();
    _mutex.unlock();

    if(selftest()){
        digitalWrite(19,HIGH);
    }
 }
template<typename RTC>
void TimeMachine<RTC>::attach(Callback<void(ExceptionType,const String)> func)
{
    _delegateCallbacks.push_back(func) ;
}
template<typename RTC>
bool TimeMachine<RTC>::selftest()
{
    int timeout= 3;
    time_t val = 0;
    do
    {
        ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
   
        val = getEpoch();
      
        if(val !=0 ){
            return true;
        }
    }while(--timeout > 0);
    
    for(auto& v : _delegateCallbacks){
        v.call(ExceptionType::RTCException,String(__FILE__)+String(":")+String(__LINE__));
    }
   
    return false;
}

template<typename RTC>
time_t TimeMachine<RTC>::getEpoch()
{
     time_t epoch=0;
     _mutex.lock();
     if(_rtc.isRunning()){
       epoch = _rtc.getEpoch();
     }
    _mutex.unlock();
    return epoch;
}
template<typename RTC>
void TimeMachine<RTC>::setEpoch(time_t epoch)
{
     _mutex.lock();
     _rtc.setEpoch(epoch);
    _mutex.unlock();
}

template<typename RTC>
String TimeMachine<RTC>::getDateTime()
{
     
     _mutex.lock();
     String datetime="";
     if(_rtc.isRunning()){
        datetime = _rtc.getDateTime(true);
     }
    _mutex.unlock();
    return datetime;
}
/*
template<>
class TimeMachine<RTCBase>
{
public:
    TimeMachine()=delete;
 
    TimeMachine(RTCBase* rtc,rtos::Mutex& mutex):_thunk_rtc(rtc),_rtc(*_thunk_rtc),_mutex(mutex)
    {
        //_rtc=(*_thunk_rtc);
        _rst = 0;
    }
    TimeMachine(RTCBase* rtc,rtos::Mutex& mutex,uint8_t rst):_thunk_rtc(rtc),_rtc(*_thunk_rtc),_mutex(mutex)
    {
        //_rtc=(*_thunk_rtc);
        _rst = rst;
    }
    ~TimeMachine()=default;
    void startup(bool pwrEnable)
    {
         if(pwrEnable){
            pinMode(_rst,OUTPUT);
            digitalWrite(_rst,HIGH);
            ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
        }
        _mutex.lock();
        _rtc.begin();
        _rtc.setDateTime(__DATE__,__TIME__);
       // _rtc.setEpoch(1610000000);
        _rtc.setHourMode(CLOCK_H24);
        _rtc.startClock();
        _mutex.unlock();
    }
    time_t getEpoch()
    {
        time_t epoch=0;
        _mutex.lock();
        if(_rtc.isRunning()){
            epoch = _rtc.getEpoch();
        }
        _mutex.unlock();
        return epoch;
    }
    String getDateTime()
    {
         _mutex.lock();
        String datetime="";
        if(_rtc.isRunning()){
            datetime = _rtc.getDateTime(true);
        }
        _mutex.unlock();
        return datetime;
    }
    void setEpoch(time_t epoch)
    {
         _mutex.lock();
        _rtc.setEpoch(epoch);
        _mutex.unlock();
    }
private:
    RTCBase* _thunk_rtc;
    RTCBase& _rtc;
    rtos::Mutex& _mutex;
    uint8_t _rst;
};

template class TimeMachine<RTCBase>;
*/
template class TimeMachine<DS1307>;

