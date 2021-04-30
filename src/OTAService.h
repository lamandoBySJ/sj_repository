#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H
#include "arduino.h"
#include "platform_debug.h"
#include <mutex>
#include <thread>
#include <functional>
#include <HTTPClient.h>
#include <Esp32httpUpdate.h>
class OTAService
{
public:
    OTAService(){
      
    }
    ~OTAService(){

    }
    void  execute(const String url);
private:
   
};

#endif