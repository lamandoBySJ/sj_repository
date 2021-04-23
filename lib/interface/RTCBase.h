#ifndef RTCBASE_H
#define RTCBASE_H

class RTCBase
{
public:
     
   virtual String& getDateTime(bool duplicate=false) = 0;
   virtual bool begin() = 0; 
   virtual bool isRunning(void)  = 0;
   virtual void startClock(void)  = 0;
   virtual void stopClock()  = 0;
   virtual void setHourMode(uint8_t h_mode)  = 0;
   virtual uint8_t getHourMode()  = 0;

   virtual void setMeridiem(uint8_t meridiem)  = 0;
   virtual uint8_t getMeridiem() = 0;

   virtual void setSeconds(uint8_t second)  = 0;
   virtual void setMinutes(uint8_t minute)  = 0;
   virtual void setHours(uint8_t hour)  = 0;
   virtual void setDay(uint8_t day)  = 0;
   virtual void setWeek(uint8_t week)  = 0;
   virtual void setMonth(uint8_t month) = 0;
   virtual void setYear(uint16_t year) = 0;

   virtual void setDate(uint8_t day, uint8_t month, uint16_t year) = 0;
   virtual void setTime(uint8_t hour, uint8_t minute, uint8_t second) = 0;

   virtual void setDateTime(const char* date,const char* time) = 0;

   virtual uint8_t getSeconds() = 0;
   virtual uint8_t getMinutes() = 0;
   virtual uint8_t getHours() = 0;
   virtual uint8_t getDay() = 0;
   virtual uint8_t getWeek() = 0;
   virtual uint8_t getMonth() = 0;
   virtual uint16_t getYear() = 0;

   virtual void setEpoch(time_t epoch) = 0;
   virtual time_t getEpoch() = 0;

   virtual void setOutPin(uint8_t mode) = 0;
   virtual bool isOutPinEnabled() = 0;
   virtual bool isSqweEnabled() = 0; 
};

#endif