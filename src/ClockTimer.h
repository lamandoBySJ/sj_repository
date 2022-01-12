#pragma once

#include "Countdown.h"
#include "app/RTC/RTC.h"
#include "AsyncMqttClientService.h"

class TimeSyncBuffer
{
  public:
  TimeSyncBuffer(){
      _payloadTimeSync = "{\"DeviceID\":\""+Device::WiFiMacAddress();
      _payloadTimeSync+="\",\"version\":\""+platformio_api::get_version();
      _payloadTimeSync+="\",\"version_code\":"+String(1,DEC);
      _payloadTimeSync+=",\"wifi_channel\":"+String(1,DEC)+"}";
  }
  ~TimeSyncBuffer()=default;
  static TimeSyncBuffer& GetInstance(){
    static TimeSyncBuffer* buffer=new TimeSyncBuffer();
    return *buffer;
  }
  static String& Payload(){
    return GetInstance()._payloadTimeSync;
  }

private:
String _payloadTimeSync;
};

class ClockTimer : public TimeoutCountup
{
public:
    ClockTimer():TimeoutCountup(0,30,"ClockTimer")
    {

    }
    ~ClockTimer()=default;
    void onTimeout(){

        rtc_epoch = RTC::LocalDateTime::now(rtc_datetime);
        PlatformDebug::printf("\nrtc_epoch          : %d,%s\n", rtc_epoch,rtc_datetime.c_str());
        PlatformDebug::printf("\nepoch              : %d,%s\n", counter(),datetime().c_str());
        if( abs(rtc_epoch-counter()) < 10){
            syncCounter(rtc_epoch);
        }else{
            if(++CNT == 10){
                CNT=0;
                AsyncMqttClientService::publish("SmartBox/TimeSync",TimeSyncBuffer::Payload());
            }
        }
        system_clock_epoch = SystemClock::LocalDateTime::now(system_clock_datetime);
        PlatformDebug::printf("\nsystem_clock_epoch : %d,%s\n",system_clock_epoch,system_clock_datetime.c_str());

        //if( abs(EpochRecorder::getEpoch()-rtc_epoch) > 60 ){
                //PlatformDebug::printf("\n1----------------rtc_epoch %d,%s----------------\n",rtc_epoch,rtc_datetime.c_str());
              //  system_clock_epoch = SystemClock::LocalDateTime::now(system_clock_datetime);
               // doc["system_clock_epoch"]= system_clock_epoch;
                //doc["system_clock_datetime"]= system_clock_datetime;  
               // rtc_epoch = system_clock_epoch;
               // rtc_datetime = system_clock_datetime;
        //}
        reset();
    }
    String& datetime(){
       Convert::ToLocalDateTime(counter(),rtc_datetime);
       return  rtc_datetime;
    }
    time_t epoch(){
         return counter();
    }
private:

    String rtc_datetime;
    String system_clock_datetime;
  
    time_t rtc_epoch;
    time_t system_clock_epoch;
    time_t CNT=0;
};

class ClockTimerRecorder
{
public:
	ClockTimerRecorder()=default;
	~ClockTimerRecorder()=default;
	static time_t Epoch(){
		return GetInstance()._clockTimer.epoch();
	}
    static String& Now(){
		return GetInstance()._clockTimer.datetime();
	}
	static void SyncTime(time_t epoch){
		 GetInstance()._clockTimer.syncCounter(epoch);
	}
	static ClockTimerRecorder& GetInstance(){
		static ClockTimerRecorder recorder;
		return recorder;
	}
    static ClockTimer& GetClockTimer(){
        return  GetInstance()._clockTimer;
    }
private:
ClockTimer _clockTimer;
};