#pragma once
#include "app/RTC/RTCBase.h"
#include "Logger.h"

class TimeDirector 
{
public:
	TimeDirector()=delete;
	~TimeDirector()=delete;
	//创建框架(稳定)
 	static bool generateClock(RTCBase* builder)
	{
		*RTCBase::getInstance()=builder;
		int retry_count=3;
		bool ok =false;
		do
		{   
			builder->power_on();
			ok = builder->begin(4,15,100000);
			ok ? delay(100) : Logger::error(__PRETTY_FUNCTION__,__LINE__);
			ok = builder->init(CLOCK_H24); 
			ok ? delay(100) : Logger::error(__PRETTY_FUNCTION__,__LINE__);
			ok = builder->setDateTime(__DATE__,__TIME__); 
			ok ? delay(100) : Logger::error(__PRETTY_FUNCTION__,__LINE__);	
		}while(ok==false && retry_count-- > 0);
		//ok?Serial.println("--- Clock OK---"):Serial.println("--- Clock ERROR ---");
		return ok;
	}

};
