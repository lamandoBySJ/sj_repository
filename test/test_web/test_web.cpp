#include <Arduino.h>
#include <unity.h>
#include "ESPwebServer.h"
#include "rtos/Queue.h"
#include "rtos/Mail.h"
#include "rtos/MemoryPool.h"
#include "rtos/Thread.h"
using namespace std;
using namespace mstd;
using namespace rtos;
using namespace platform_debug;

#define BAND    470E6 

Thread threadWeb(osPriorityNormal,1024*6,NULL,"web");
Thread threadTest(osPriorityNormal,1024*6,NULL,"Test");
Thread threadSignal(osPriorityNormal,1024*6,NULL,"signal");

struct mail_control_t{
  uint32_t counter=0; 
  uint32_t id=0;   
};
rtos::Mail<mail_control_t, 16> mail_box;
class Test
{
public:
      std::mutex _mtx;
      Test(){
           Serial.println("structor");
      }
       ~Test(){
        Serial.println("destructor");
      }
  void run(){
    while(true){
      std::lock_guard<std::mutex> lck(_mtx);
      Test test;
      static int ct=6;
      while(--ct){
          std::this_thread::sleep_for(std::chrono::seconds(1));
      }
       ct=6;
      Serial.println("AAAAAAAAAAAAAAATesting......");
      //String(ThisThread::flags_wait_all_for(0x1,std::chrono::seconds(60),true),DEC));
    
    }
  }
  void run2(){
    while(true){
     // Serial.println("BBBBBBBBBBBBBBTesting......");
      ThisThread::flags_wait_any_for(0x2,std::chrono::seconds(60),true);
      //std::this_thread::sleep_for(std::chrono::seconds(3));
      dummy();
    }
  }
  void dummy(){
    std::lock_guard<std::mutex> lck(_mtx);
    Serial.println("......dummy......");
  }
  void siganl(){
    while(1){
      osEvent evt=  mail_box.get();
      if (evt.status == osEventMail) {
        /*if(!ESPwebServer.isRunning()){
          threadWeb.start(callback(&ESPwebServer,&ESPWebServer::startup));
          threadWeb.join();
        }*/

       
        mail_control_t* mail= (mail_control_t*)evt.value.p;
        //threadWeb.flags_set(2);
        Serial.println(String(threadTest.flags_set(2),DEC)) ;
        Serial.println("set:"+String(mail->id,DEC));
        mail_box.free(mail);
      }
    }
  }
};

Test test;
std::thread thd;
//Thread threads[2];



//ESPWebServer ESPwebServer;
//RGBCollector<BH1749NUC> RGBcollector(MQTTnetwork,colorSensor);

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    UNITY_BEGIN();    // IMPORTANT LINE!

    esp_sleep_wakeup_cause_t cause =  esp_sleep_get_wakeup_cause();
    #ifdef NDEBUG
        Heltec.begin(false, true , false , true, BAND);
        Serial.setDebugOutput(false);
    #else
        #ifdef OLEDSCREEN
        Heltec.begin(true, true , true , true, BAND);
        // PlatformDebug::init(Serial,oled);
        PlatformDebug::init(Serial,OLEDScreen<12>(Heltec.display));
        // PlatformDebug::init(Serial,std::move(oled));
        PlatformDebug::printLogo();
        
        #else
        Heltec.begin(false, false , true , true, BAND);
        PlatformDebug::init(Serial);
        #endif
    #endif

    attachInterrupt(0,[](){
    volatile static int id=0;
    mail_control_t* mail=  mail_box.alloc();
    mail->id = ++id;
    mail_box.put_from_isr(mail);
  },FALLING);

  //  ESPwebServer.startup();

    //thd = std::thread(&Test::run,&test);
  Serial.println("ThreadStart");
  threadWeb.start(callback(&test,&Test::run));
  threadTest.start(callback(&test,&Test::run2));
  threadSignal.start(callback(&test,&Test::siganl));

  threadWeb.join();
  threadSignal.join();

  platform_debug::PlatformDebug::pause();
}

uint8_t i = 0;
uint8_t max_blinks = 5;

void loop() {

    osEvent evt=  mail_box.get();
    
    if (evt.status == osEventMail) {
      /*if(!ESPwebServer.isRunning()){

        threadWeb.start(callback(&ESPwebServer,&ESPWebServer::startup));
        threadWeb.join();
      }*/
      mail_control_t* mail= (mail_control_t*)evt.value.p;
      threadWeb.flags_set(mail->id);
      Serial.println("set:"+String(mail->id,DEC));
      mail_box.free(mail);
    }
     UNITY_END(); // stop unit testing
}