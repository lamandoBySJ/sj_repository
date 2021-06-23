#include <heltec.h>
//#include <SPI.h>
//#include <MFRC522.h>
#include "esp_sleep.h"
#include "LoopTaskGuard.h"
#include "IPSBox.h"
#include "platform_debug.h"
#include "Logger.h"
#include "app/RTC/RTC.h"

using namespace std;
#define BAND    470E6 

//python3 esptool.py --port COM20 --baud 115200 write_flash -fm dio -fs 16MB  0x410000 partitions.bin
//C:\Users\Administrator\.platformio\packages\framework-arduinoespressif32\tools\partitions


/*libraries download:
1.ESP32httpUpdate
2.ESPAsyncWebServer
*/


IPSBox* box=new IPSBox();

#define OLEDSCREEN 
void setup() {
 // put your setup code here, to run once:
  #ifdef NDEBUG
    //WIFI Kit series V1 not support Vext control
    Heltec.begin(false, true , false , true, BAND);
    Serial.setDebugOutput(false);
    //LoRa.dumpRegisters(Serial);
  #else
    #ifdef OLEDSCREEN
        Heltec.begin(true, true , true , true, BAND);
       // PlatformDebug::init(Serial,oled);
        PlatformDebug::getInstance()->init(Serial,OLEDScreen<12>(Heltec.display));
        PlatformDebug::printLogo();
    #else
      Heltec.begin(false, false , true , true, BAND);
      PlatformDebug::getInstance()->init(Serial);
    #endif
  #endif


  try
  {
     //box->wire1_join_to_i2c_bus(32,33);
     box->platformio_init();
     box->i2c_devices_init();
    // box->lora_gateway_init();
    //box->lora_collector_init();
    //ox->lora_beacon_init();
    box->lora_tag_init();
  }catch(os::alloc_error& e){
    TracePrinter::printTrace(e.what());
    Logger::error(__FUNCTION__,__LINE__);
  }

}

void loop() {
  
  PlatformDebug::println(" ----------- IPS ----------- ");
  String tm;
  SystemClock::SyncTime(1624431378);
  for(;;){
    delay(1000);
    SystemClock::UniversalTime::now(tm);
    TracePrinter::printTrace("SystemClockTime:"+tm);
  }
  while (true) {
    switch (guard::LoopTaskGuard::getLoopTaskGuard().get_signal_id())
    {
     case 0:
          {
              box->start_web_service();
          }
       break;
     case 1:
          { 
              
              
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
//PlatformDebug::pause();
//std::this_thread::sleep_for(chrono::seconds(10));
//  std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
//  lck.lock();
// std::lock_guard<rtos::Mutex> lck(_mtx);
