#include <heltec.h>
#include <app/TimeMachine/TimeMachine.h>
//#include <SPI.h>
//#include <MFRC522.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
  #include "freertos/task.h"
  #include "freertos/queue.h"
}
#include "esp_sleep.h"
#include "SmartBox.h"
#include "LoopTaskGuard.h"

using namespace std;
#define BAND    470E6 
//python3 esptool.py --port COM20 --baud 115200 write_flash -fm dio -fs 16MB  0x410000 partitions.bin
//C:\Users\Administrator\.platformio\packages\framework-arduinoespressif32\tools\partitions
//DS1307 ds1307(Wire1,32,33); //ips
std::mutex std_mutex;
rtos::Mutex stdMutex;
SmartBox* smartBox=new SmartBox();
//#define OLEDSCREEN 
void setup() {
 // put your setup code here, to run once:
  #ifdef NDEBUG
    //WIFI Kit series V1 not support Vext control
    Heltec.begin(false, false , false , true, BAND);
    Serial.setDebugOutput(false);
    //LoRa.dumpRegisters(Serial);
  #else
    #ifdef OLEDSCREEN
      Heltec.begin(true, true , true , true, BAND);
       // PlatformDebug::init(Serial,oled);
        PlatformDebug::getInstance()->init(Serial,OLEDScreen<12>(Heltec.display));
        // PlatformDebug::init(Serial,std::move(oled));
        PlatformDebug::printLogo();
    #else
      Heltec.begin(false, false , true , true, BAND);
      PlatformDebug::getInstance()->init(Serial);
    #endif
  #endif
  try
  {
    smartBox->platformio_init();
  }catch(os::alloc_error& e){
    PlatformDebug::println(e.what());
    PlatformDebug::pause();
  }catch(std::exception& e)
  {

  }
  
}

void loop() {
  
  PlatformDebug::println(" ----------- STLB ----------- ");
  smartBox->startup();
  while (true) {
    switch (guard::LoopTaskGuard::getLoopTaskGuard().get_signal_id())
    {
     case 0:
          {
              smartBox->start_web_service();
          }
       break;
     case 1:
          { 
              smartBox->color_measure();
          }
      break;
     case 2:
          {
              
          }
      break;
     case 3:
          {
            
          }
      break;
     default:
      break;
    }
  }
}
//  std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
//  lck.lock();
// std::lock_guard<rtos::Mutex> lck(_mtx);
