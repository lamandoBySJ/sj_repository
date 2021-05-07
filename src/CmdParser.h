#ifndef CMD_PARSER_H
#define CMD_PARSER_H
#include "Arduino.h"
#include <ArduinoJson.h>
#include "OTAService.h"
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <map>
#include "StringHelper.h"
#include "platform/mbed.h"
#include "DS1307.h"
#include "app/TimeMachine/TimeMachine.h"
#include "Countdown.h"
#include "platform_debug.h"
using namespace std;
extern TimeMachine<DS1307> timeMachine;

enum class RequestType : uint8_t
{
  ALS_MEASURE = 1,
  MANUAL_REQUEST,
  OTA_UPGRADE,
  OTA_CANCEL,
  FILE_DOWNLOAD,
  FILE_DELETE,
  ESP_RESTART
};

class CmdParser
{
public:
    CmdParser():_mtx(),_OTAService()
    {
        _funOTA = std::bind(&OTAService::execute,&_OTAService,std::placeholders::_1);
        str_map_type[String("als_measure")]   = RequestType::ALS_MEASURE;
        str_map_type[String("ota_upgrade")]   = RequestType::OTA_UPGRADE;
        str_map_type[String("ota_cancel")]    = RequestType::OTA_CANCEL;
        str_map_type[String("file_download")] = RequestType::FILE_DOWNLOAD;
        str_map_type[String("file_delete")]   = RequestType::FILE_DELETE;
       // str_map_type[String("esp_restart")]   = RequestType::ESP_RESTART;
        str_map_type[String("manual_request")]= RequestType::MANUAL_REQUEST;
    }
    ~CmdParser(){

    }
    void callSmartBoxTimeSync(){

    }
    void callServerRequest(){
        
    }
    void onMessageCallback(const String& topic,const String& payload);

    void setCallbackSmoxartBTimeSync(std::function<void(time_t)> fun){
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
	    lck.lock();
        this->_funSmartBoxTimeSync=fun;
    }
    void setCallbackServerRequest(std::function<void(time_t)> fun){
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
	    lck.lock();
        this->_funSmartBoxTimeSync=fun;
    }
    using CallFunction = void(CmdParser::*)();
    
    void timeoutChecker(){
        Countdown countdown(6000);
        for(;;){
            if(countdown.expired()){
                platform_debug::PlatformDebug::println("countdown.expired: ESP.restart()");
                std::this_thread::sleep_for(std::chrono::seconds(3));
                ESP.restart();
            }else{
                platform_debug::PlatformDebug::println("countdown.left_ms:"+String(countdown.left_ms(),DEC));
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
private:
    std::thread _threadCountdown;
    std::mutex _mtx;
    OTAService _OTAService;
    StringHelper _StringHelper;
    std::vector<String> _topics;
    std::map<String,RequestType> str_map_type;
    std::function<void(const String&)> _funOTA;
    //mbed::Callback<void(time_t)> _funSmartBoxTimeSync;
    std::function<void(time_t)> _funSmartBoxTimeSync;
    std::function<void(time_t)> _funServerRequest;
    //rtos::Mail<mqtt::mail_t, 16> _mail_box;

};

#endif