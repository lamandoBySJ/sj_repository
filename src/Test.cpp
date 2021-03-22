#include "Test.h"

//TickerDataClock clock(NULL);
 //TickerDataClock::time_point today =  clock.now();
 //time_t tt= today.time_since_epoch().count();

//static system_clock::time_point today = system_clock::now();
// std::time_t tt = system_clock::to_time_t(today);
//debug( "today is:%s\n ",ctime(&tt));

//thread1.start(callback(TaskTest0));
//thread2.start(callback(TaskTest,&a));
void TaskTest0()
{
 String datetime="";
  for(;;){
     // stdmutex.lock();
      //String&& datetime = timeMachine.getDateTime();
     // debug("Test0: __cplusplus:%s , %s\n", String(__cplusplus,DEC).c_str(),datetime.c_str() );
     // ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
      //stdmutex.unlock();
  }
}
void TaskTest(int *pvParameters  )
{
 
  for(;;){
     // stdmutex.lock();
     
      //stdmutex.unlock();
  }
}
void TaskDebug( void *pvParameters )
{
  //int cnt=0;
  for(;;){
     // vTaskResume(handleTaskDebug);
      //Serial.println("hello");
    //debug_if(true,"debug_if:%d\n",data);
    //delay(10000);
   // ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
      //debug("task debug ...%d\n",++x);
      //ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  }
  
}