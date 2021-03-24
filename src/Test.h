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
    void run_test()
    {
      String topic;
      String payload;
          while(true){
            osEvent evt= _mail_box.get();
            if (evt.status == osEventMail) {
                test::mail_t *mail = (test::mail_t *)evt.value.p;
                topic = mail->topic;
                payload = mail->payload;
                _mail_box.free(mail); 
               // for(auto& v : _debugCallbacks){
              //      v.call(mail->topic,mail->payload);
              //  }
                platform_debug::TracePrinter::printTrace(mail->topic+String(":")+mail->payload);
            }
          }
    }
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
    
private:
    Thread thread;
    Mail<test::mail_t,6> _mail_box;
   // std::vector<mbed::Callback<void(const String&,const String&)>>  _debugCallbacks;
};


#endif
