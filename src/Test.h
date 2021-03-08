#ifndef TEST_H
#define TEST_H

#include <arduino.h>
#include <rtos/rtos.h>
#include <app/TimeMachine/TimeMachine.h>
//vTaskSuspend(handleTaskDebug);

extern TimeMachine<DS1307> timeMachine;


template <typename R>
class Work : private detail::CallbackBase
{
public:
  auto call() -> decltype(_call) 
  {
    //MBED_ASSERT(bool(*this));
    return _call;
  }
};

class Test
{
public:
    Test(){};
    ~Test(){};
    void run(){
      
      for(;;){
        String&& datetime = timeMachine.getDateTime();
        debug("Callback: ESP.getFreeHeap():%d ,%s\n",ESP.getFreeHeap(),datetime.c_str() );
        ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
      }
    }
 
    void startup(){
      thread.start(callback(this,&Test::run));
    }
private:
    Thread thread;
};


void TaskDebug( void *pvParameters );

#endif