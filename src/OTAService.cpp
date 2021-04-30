#include "OTAService.h"


void OTAService::execute(const String url){
      //  url="http://192.168.1.104/bin/firmware.bin";
        platform_debug::PlatformDebug::println("Start:OTA...");
            ESPhttpUpdate.rebootOnUpdate(false);
            t_httpUpdate_return ret =  ESPhttpUpdate.update(url);
            switch(ret) {
              case HTTP_UPDATE_FAILED:
                platform_debug::PlatformDebug::printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                break;
              case HTTP_UPDATE_NO_UPDATES:
                platform_debug::PlatformDebug::println("HTTP_UPDATE_NO_UPDATES");
                break;
              case HTTP_UPDATE_OK:
                platform_debug::PlatformDebug::println("HTTP_UPDATE_OK");
                break;
              default:break;
            }
       std::this_thread::sleep_for(std::chrono::seconds(3));
       ESP.restart();
}