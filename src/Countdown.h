#ifndef __COUNTDOWN_H
#define __COUNTDOWN_H

#include "Arduino.h"
#include <mutex>
#include <thread>
#include <atomic>
#include "platform_debug.h"


class Countdown
{
public:
    Countdown()=delete;
    Countdown(int ms):interval_end_ms(0),_ms(0),_mtx(),_expired(false),_timer(NULL)
    {
        /*
        _timer = timerBegin(0,80,true);//设置定时器0，80分频，向上计数

        timerAttachInterrupt(_timer,[]()->void{
     
        },true);
        
        timerAlarmWrite(_timer,1000000,true);
        timerAlarmEnable(_timer);
       countdown_ms(ms);
       */
    }
    
    ~Countdown(){

    }
    void countdown_ms(unsigned long ms)  
    {   
        if(xTimerIsTimerActive(_timer)){
            xTimerStop(_timer,0);
        }
        _ms=0;
        _expired=false;
        interval_end_ms = ms;
        xTimerStart(_timer,0);
       // xTimerReset(_timer,0);
    }

    void countdown(int seconds)
    {
        countdown_ms((unsigned long)seconds * 1000L);
    }
    int left_ms()
    {
       std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
       lck.lock();
       platform_debug::PlatformDebug::println("left_ms:"+String(_ms,DEC));
  
        return interval_end_ms - _ms;
    }
    bool expired()
    {
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
       lck.lock();
        if(_expired){
            return true; 
        }else if(_ms >= interval_end_ms){
            _expired = true;
            xTimerStop(_timer,0);
        } 
        return _expired; 
    }
    void count(void* pvTimerID){
         static_cast<Countdown*>(pvTimerID)->_ms+=1;
        platform_debug::PlatformDebug::println("2_ms:"+String(static_cast<Countdown*>(pvTimerID)->_ms,DEC));
    }
   
//private:
    static void _thunkCountdown(void* pvTimerID);
    unsigned long interval_end_ms; 
    unsigned long _ms;
    std::mutex _mtx;
    bool _expired;
    hw_timer_t *_timer;
};

#endif