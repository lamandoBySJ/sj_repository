#include <heltec.h>
//#include <ArduinoJson.h>
#include <rtos/rtos.h>
#include <TimeMachine/TimeMachine.h>

using namespace rtos;

#define BAND    433E6 
#if CONFIG_AUTOSTART_ARDUINO
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#endif

rtos::Mutex stdmutex;
rtos::Mutex std_mutex;
DS1307 RTC(Wire,32,33);
TimeMachine timeMachine(RTC,std_mutex);


Thread thread;
Thread thread1("Thd1",1024*2,1);
Thread thread2("Thd2",1024*2,2);

char data[]="hello sj~";


void TaskDebug( void *pvParameters );

void TaskTest0()
{
  int x = 0;
  for(;;){
     // stdmutex.lock();
      debug("task test0 ......%d\n",++x);
      ThisThread::sleep_for(Kernel::Clock::duration_u32(10000));
      //stdmutex.unlock();
  }
}

void TaskTest(int *pvParameters  )
{
  for(;;){
     // stdmutex.lock();
      debug("task test ......%d\n",*pvParameters );
     
      ThisThread::sleep_for(Kernel::Clock::duration_u32(10000));
      //stdmutex.unlock();
  }
}
int (*_call)();
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
        String&& rtc = RTC.getDateTime();
        debug("Callback: __cplusplus:%s , RTC:%d,%s\n", String(__cplusplus,DEC).c_str(),(int32_t)RTC.getEpoch(),rtc.c_str() );
        ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
      }
    }
 
    void startup(){
      thread.start(callback(this,&Test::run));
    }
private:
  
};
  int a =1993;
Test test;
Test* t;
void setup() {
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  //LoRa.dumpRegisters(Serial);

  timeMachine.startup(NULL);

  /*
  xTaskCreatePinnedToCore(
    TaskDebug
    ,  "TaskDebug"
    ,  8*1024  
    ,  NULL
    ,  1  
    ,  &handleTaskDebug
    ,  ARDUINO_RUNNING_CORE);
  attachInterrupt(0, []  {
    vTaskSuspend(handleTaskDebug);
  }, FALLING);   

  */
  String&& debugtime=RTC.getDateTime();
  

  debug("RTC:%d,%s\n",(int)RTC.getEpoch(),debugtime.c_str());
  
  
  //Work<int> work;
  //work.call();
  ThisThread::sleep_for(Kernel::Clock::duration_u32(3000));
  test.startup();
  thread1.start(callback(TaskTest0));
  thread2.start(callback(TaskTest,&a));


}
int cnt=0;
void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("hello");
  //debug_if(true,"debug_if:%d\n",data);
  //delay(10000);
 // ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  if(++cnt==10){
    cnt=0;
   // vTaskResume(handleTaskDebug);
  }
  //debug("%s:%d\n",data,cnt);
  vTaskDelete(NULL);
}

void TaskDebug( void *pvParameters )
{
  //int x = 0;
  for(;;){
      //debug("task debug ...%d\n",++x);
      //ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  }
}


