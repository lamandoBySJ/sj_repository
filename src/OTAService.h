#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H
#include "arduino.h"
#include "platform_debug.h"
#include <mutex>
#include <thread>
#include <functional>
#include <HTTPClient.h>
#include <Esp32httpUpdate.h>
#include <functional>
class OTAService
{
public:
    OTAService(){
      
    }
    ~OTAService(){

    }
    void onMessageMqttCallback(const String& topic,const String& payload);
    void  execute(const String url);
private:
   
};

#endif