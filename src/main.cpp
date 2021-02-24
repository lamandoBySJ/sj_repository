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


rtos::Mutex std_mutex;
DS1307 RTC(Wire,32,33);
TimeMachine timeMachine(RTC,std_mutex);


Thread thread;
char data[]="hello sj~";
xTaskHandle handleTaskDebug=NULL;
void TaskDebug( void *pvParameters );
void TaskTest( void *pvParameters );
class Test{
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
};

Test test;

void setup() {
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  LoRa.dumpRegisters(Serial);
}
/*
   vTaskSuspendAll();       // 开启调度锁      
        printf("任务vTaskLed1正在运行\r\n");   
        if(!xTaskResumeAll())      // 关闭调度锁，如果需要任务切换，此函数返回pdTRUE，否则返回pdFALSE 
        {
            taskYIELD ();
        }    
*/
void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("hello");
  debug_if(true,"debug_if:%s\n",data);
  debug("%s\n",data);
  //std_mutex.lock();
  //delay(10000);
  ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));


  //LoRa.dumpRegisters(Serial);

  timeMachine.startup(NULL);

 
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

  
  String&& debugtime=RTC.getDateTime();
  debug("RTC:%d,%s\n",(int)RTC.getEpoch(),debugtime.c_str());
 
  test.startup();
  
  // put your main code here, to run repeatedly:
  //Serial.println("hello");
  //debug_if(true,"debug_if:%d\n",data);
  //delay(10000);
  ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  //vTaskResume(handleTaskDebug);
  debug("%s\n",data);
  vTaskDelete(NULL);
}

void TaskDebug( void *pvParameters )
{
  int cnt=0;
  for(;;){
      debug("task debug ... %d\n",++cnt);
      ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  }
}
