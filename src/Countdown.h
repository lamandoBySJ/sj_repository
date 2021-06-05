#ifndef TIMEOUT_CHECKER_H
#define TIMEOUT_CHECKER_H
#include "platform_debug.h"
#include "freertos/timers.h"
#include <atomic>
#include "hal/us_ticker_api.h"

namespace countdown
{
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
class TimerBase
{
public:
    static void _thunk_countdown(void* pvTimerID)
    {   
       // portENTER_CRITICAL_ISR(&mux);
       // static_cast<Timer*>(pvTimerID)->task_countdown();
         us_ticker_irq_handler();
       //portEXIT_CRITICAL_ISR(&mux);
    }  
    TimerBase(){
              interval_end_ms=0;
                _max=numeric_limits<unsigned long>::max();
        _countdownTimer = xTimerCreate("countdownTimer", pdMS_TO_TICKS(1000), pdTRUE,this, reinterpret_cast<TimerCallbackFunction_t>(&TimerBase::_thunk_countdown));    
            start();
    }
     void start(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        xTimerStart(_countdownTimer,0);
    }
    void stop(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        xTimerStop(_countdownTimer,0);
    }
    void reset(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        interval_end_ms=0;
    }
    unsigned long  read_ms(){
        Serial.println("read_ms"+String(interval_end_ms,DEC));
        return interval_end_ms;
    }
private:
    TimerHandle_t _countdownTimer;
    rtos::Mutex _mtx;
    unsigned long  interval_end_ms;
    std::atomic<unsigned long> _max;
};

class Timer : public TimerBase
{
public:
    
    Timer()
    {
        Serial.printf("constructor:%p\n",this);
    }
    
private:
    std::atomic<bool> _will_overflow;
};
}
class Countdown
{
public:

   explicit Countdown(int ms)
   {
  
        countdown_ms(ms);
    }

    void countdown_ms(unsigned long ms)  
    {
        interval_end_ms = ms;
    }
    void countdown(int seconds)
    {
        countdown_ms((unsigned long)seconds * 1000L);
    }
    //void startup();
    //void task_restart_countdown();

    void shutdown();

    bool expired()
    {
        return t.read_ms() >= interval_end_ms;
    }
    int left_ms()
    {
        return interval_end_ms - t.read_ms();
    }

private:
   // rtos::Mail<int,2> _mail_box;
   // rtos::Thread _thread;
    countdown::Timer t;
    unsigned long interval_end_ms; 
    
    
};
#endif