#pragma once

#include "Arduino.h"
#include "DS1307.h"
#include "platform_debug.h"
#include "app/RTC/TimeDirector.h"
#include <sys/time.h>
#include <time.h>
#include "lwip/apps/sntp.h"
template<typename T>
class UniversalTime 
{
public:
	UniversalTime()=delete;
	~UniversalTime()=delete;

	static time_t now(String& nowtime)
	{
		time_t epoch = T::now();
		Convert::ToDateTime(epoch,nowtime);
		return epoch;
	}

};
template<typename T>
class LocalDate 
{
public:
	LocalDate()=delete;
	~LocalDate()=delete;
		
	static time_t now(String& nowtime)
	{
		time_t epoch = T::now();
		Convert::ToLocalDate(epoch,nowtime);
		return epoch;
	}

};
template<typename T>
class LocalTime 
{
public:
	LocalTime()=delete;
	~LocalTime()=delete;
	static time_t now(String& nowtime)
	{
		time_t epoch = T::now();
		Convert::ToLocalTime(epoch,nowtime);
		return epoch;
	}

};

template<typename T>
class LocalDateTime 
{
public:
	LocalDateTime()=delete;
	~LocalDateTime()=delete;
	static time_t now(String& nowtime)
	{
		time_t epoch = T::now();
		Convert::ToLocalDateTime(epoch,nowtime);
		return epoch;
	}
};

template<typename T>
class ZonedDateTime 
{
public:
	ZonedDateTime()=delete;
	~ZonedDateTime()=delete;
	static time_t now(String& nowtime)
	{
		time_t epoch = T::now();
		Convert::ToZonedDateTime(epoch,nowtime);
		return epoch;
	}

};
					
class SystemClock : virtual public UniversalTime<SystemClock>
						,virtual public LocalDate<SystemClock>
						,virtual public LocalTime<SystemClock>
						,virtual public LocalDateTime<SystemClock>
						,virtual public ZonedDateTime<SystemClock>
{
public:
	SystemClock()=delete;
	~SystemClock()=delete;
	static time_t now() 
	{
		//struct timeval tv;
		//gettimeofday(&tv, NULL);
		//return tv.tv_sec;
		struct tm t;
    	getLocalTime(&t, 0);
		time_t tt = mktime(&t);
		return tt;
	}
	static void SyncTime(time_t t)
	{
	
		struct timeval tv;
		tv.tv_sec = t;
		tv.tv_usec = 0;
		//timezone tz;
		//tz.tz_dsttime = 8;
		//tz.tz_minuteswest=0;
		settimeofday(&tv,NULL);
	}
private:

};


class RTC : virtual public UniversalTime<RTC>
            ,virtual public LocalDate<RTC>
            ,virtual public LocalTime<RTC>
            ,virtual public LocalDateTime<RTC>
            ,virtual public ZonedDateTime<RTC>

            
{
public:
    RTC()=delete;
    explicit RTC(const RTC&)=delete;
    explicit RTC(RTC&&)=delete;
    RTC& operator=(const RTC&)=delete;
    RTC& operator=(RTC&&)=delete;
	~RTC()=delete;

	template<typename T,typename std::enable_if_t<std::is_lvalue_reference<T>::value,int> =0>
    static bool BuildSchedule(T&& t){
        *RTCBase::getInstance() = &t;
        return TimeDirector::generateClock(*RTCBase::getInstance());
    }
    template<typename T,typename std::enable_if_t<!std::is_lvalue_reference<T>::value,int> =0>
    static bool BuildSchedule(T&& t){
        *RTCBase::getInstance() = new std::remove_reference_t<T>(std::forward<T>(t));
        return TimeDirector::generateClock(*RTCBase::getInstance());
    }
   
	static time_t now() 
	{
		return Builder().epoch();
	}
	static void SyncTime(time_t t)
	{
		Builder().setEpoch(t);
	}
//private:
	static RTCBase& Builder()
	{
        return **RTCBase::getInstance();
    }
};

