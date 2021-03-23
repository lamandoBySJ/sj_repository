#include "Test.h"

//TickerDataClock clock(NULL);
 //TickerDataClock::time_point today =  clock.now();
 //time_t tt= today.time_since_epoch().count();

//static system_clock::time_point today = system_clock::now();
// std::time_t tt = system_clock::to_time_t(today);
//debug( "today is:%s\n ",ctime(&tt));

//thread1.start(callback(TaskTest0));
//thread2.start(callback(TaskTest,&a));
void Test::run_test()
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
            for(auto& v : _debugCallbacks){
                v.call(mail->topic,mail->payload);
            }
            
        }
      }
}

//void TaskDebug( void *pvParameters );
