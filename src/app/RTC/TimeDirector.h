#pragma once
#include "app/RTC/RTCBase.h"
#include "Logger.h"
#include "platform_debug.h"

class TimeDirector 
{
public:
	TimeDirector()=delete;
	~TimeDirector()=delete;

 	static bool generateClock(RTCBase* builder)
	{
		*RTCBase::getInstance()=builder;
		int retry_count=3;
		bool ok[] ={false,false,false};
		do
		{   
			builder->power_on();

			ok[0] = builder->begin(21,22,100000);

			ok[1] = builder->init(CLOCK_H24); 

			ok[2] = builder->setDateTime(__DATE__,__TIME__); 

			if(ok[0]&&ok[1]&&ok[2]){
				PlatformDebug::println("--- Clock OK---");
				return true;
			}else{
				Logger::error(__PRETTY_FUNCTION__,__LINE__);	
			} 
		}while(retry_count-- > 0);
		
		return false;
	}
	/*
	static bool adjustClock(RTCBase* builder)
	{
		*RTCBase::getInstance()=builder;
		int retry_count=3;
		bool ok =false;
		do
		{   
			builder->power_on();
			ok = builder->begin(21,22,100000);
			
			ok = builder->init(CLOCK_H24);
			
			if(ok){
				PlatformDebug::println("--- adjustClock OK---");
			}else{
				Logger::error(__PRETTY_FUNCTION__,__LINE__);
				delay(100);	
			} 
		}while(ok==false && retry_count-- > 0);
		
		return ok;
	}*/
};
