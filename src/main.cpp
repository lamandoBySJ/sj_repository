#include <heltec.h>
//#include <ArduinoJson.h>
#include <rtos/rtos.h>
#include <app/TimeMachine/TimeMachine.h>
#include <app/ColorSensor/ColorSensor.h>
#include <ColorSensorBase.h>

using namespace rtos;

#define BAND    433E6 
#if CONFIG_AUTOSTART_ARDUINO
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#endif


rtos::Mutex std_mutex;
DS1307 RTC(Wire,32,33);
TimeMachine<DS1307> timeMachine(RTC,std_mutex);
TimeMachine<RTCBase> timeMachine2(&RTC,std_mutex);

rtos::Mutex mutex;
BH1749NUC bh1749nuc(Wire1,21,22,100000);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,mutex);
ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);

Thread thread3("Thd3",1024*2,3);
Thread thread4("Thd4",1024*2,4);
Thread thread5("Thd5",1024*2,5);
Thread thread6("Thd6",1024*2,6);
Thread thread7("Thd7",1024*2,7);

Thread thread1("Thd1",1024*2,1);
Thread thread2("Thd2",1024*2,2);

char data[]="hello sj~";

//Serial.println(uxTaskPriorityGet(myTask));configMAX_PRIORITIES

void TaskDebug( void *pvParameters );

void TaskTest0()
{
 String datetime="";
  for(;;){
     // stdmutex.lock();
      String&& datetime = timeMachine.getDateTime();
      debug("Test0: __cplusplus:%s , %s\n", String(__cplusplus,DEC).c_str(),datetime.c_str() );
      ThisThread::sleep_for(Kernel::Clock::duration_u32(10000));
      //stdmutex.unlock();
  }
}

void TaskTest(int *pvParameters  )
{
 
  for(;;){
     // stdmutex.lock();
      String&& datetime = timeMachine.getDateTime();
      debug("Test1: __cplusplus:%s , %s\n", String(__cplusplus,DEC).c_str(),datetime.c_str() );
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
        String&& datetime = timeMachine.getDateTime();
        debug("Callback: ESP.getFreeHeap():%d ,%s\n",ESP.getFreeHeap(),datetime.c_str() );
        ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
      }
    }
 
    void startup(){
      thread.start(callback(this,&Test::run));
    }
private:
    Thread thread;
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
  int timeout= 3;
  do
  {
    ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
    if(timeout-- == 0){
        //todo send message
        debug("RTC ERROR,please check out RTC DS1307,cplusplus:%ld\n",__cplusplus);
        break;
    }
  }while(timeMachine.getEpoch()==0);

  timeMachine.setEpoch(1614236396+8*60*60);
  String&& debugtime=RTC.getDateTime();
  debug("RTC:%d,%s\n",(int)RTC.getEpoch(),debugtime.c_str());

  ThisThread::sleep_for(Kernel::Clock::duration_u32(3000));
  test.startup();
 /* thread1.start(callback(TaskTest0));
  thread2.start(callback(TaskTest,&a));

  thread3.start(callback(TaskTest0));
  thread4.start(callback(TaskTest0));
  thread5.start(callback(TaskTest0));
  thread6.start(callback(TaskTest0));
  thread7.start(callback(TaskTest0));*/

}

void loop() {
  // put your main code here, to run repeatedly:
  debug("%s,__cplusplus:%ld\n",data,__cplusplus);
  vTaskDelete(NULL);
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
  


