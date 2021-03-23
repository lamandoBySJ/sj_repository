#ifndef TEST_H
#define TEST_H

#include <Arduino.h>
#include <rtos/rtos.h>
#include <app/TimeMachine/TimeMachine.h>
#include "platform_debug.h"
//vTaskSuspend(handleTaskDebug);


//extern TimeMachine<DS1307> timeMachine;
namespace test
{  
  typedef struct {
    uint32_t counter;   
    String topic;
    String payload;
  } mail_t;
}

class Test
{
public:
    Test(){};
    ~Test(){};
    void run(){
      for(;;){
      //  String&& datetime = timeMachine.getDateTime();
       // debug("Callback: ESP.getFreeHeap():%d ,%s\n",ESP.getFreeHeap(),datetime.c_str() );
        ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
      }
    }
    void run_test();
    void startup(){
      //thread.start(callback(this,&Test::run));
      thread.start(callback(this,&Test::run_test));
    }

    void onMessageCallback(const String& topic,const String& payload)
    {
        test::mail_t *mail =  _mail_box.alloc();
        if(mail!=NULL){
            mail->topic = String(topic);
            mail->payload = String(payload);
            _mail_box.put(mail) ;
        }
    }
    
     void attach(Callback<void(const String&,const String&)> func)
     {
        _debugCallbacks.push_back(func);
     }
private:
   
    Thread thread;
    Mail<test::mail_t,6> _mail_box;
    std::vector<mbed::Callback<void(const String&,const String&)>>  _debugCallbacks;
};


#endif
