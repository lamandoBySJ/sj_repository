/* ------------------------------------------------------------
 * "THE BEERWARE LICENSE" (Revision 42):
 * <cvmanjoo@gmail.com> wrote this code. As long as you retain this
 * notice, you can do whatever you want with this stuff. If we
 * meet someday, and you think this stuff is worth it, you can
 * buy me a beer in return.
 * ------------------------------------------------------------
 * ------------------------------------------------------------
 * RTC.h - Library to set & get time from I2C RTCs for Arduino
 * Created by Manjunath CV. July 08, 2017, 02:18 AM
 * Released into the public domain.
 * -----------------------------------------------------------*/

#ifndef DS1307_H
#define DS1307_H

//define all the rtc clocks.
//#define DS1307_H
//#define DS3231_H
//#define PCF8563_H

#define CLOCK_H24 0
#define CLOCK_H12 1

#define HOUR_AM 0
#define HOUR_PM 1
#define HOUR_24 2

#define SQW001Hz 10
#define SQW01kHz 11
#define SQW04kHz 14
#define SQW08kHz 18
#define SQW32kHz 32

#include <Arduino.h>
#include <Wire.h>
#include <array>
/*
 * DateTime Class
 * */
 
#define DS1307_ADDR 0x68

class Convert
{
public:
static void ToDateTime(time_t in_timestamp,String& out_datetime,int32_t timezone_offset=28800){
  time_t epochTimeZone = in_timestamp + timezone_offset;
  struct tm *_tm_zone = gmtime(&epochTimeZone);
  out_datetime = String(_tm_zone->tm_year+1900,DEC) + 
          String("-")+ 
          String(_tm_zone->tm_mon+1,DEC)+
          String("-")+ 
          String(_tm_zone->tm_mday,DEC)+
          String(" ")+ 
          String(_tm_zone->tm_hour,DEC)+
          String(":")+ 
          String(_tm_zone->tm_min,DEC)+
          String(":")+ 
          String(_tm_zone->tm_sec,DEC);
}
static time_t ToEpoch(struct tm *epoch)
{
  return  mktime(epoch);
}
};
class DS1307 //: public RTCBase
{
    public:
        using callbackFun=uint8_t(DS1307::*)();
        DS1307() = delete;
        explicit DS1307(TwoWire& wire):_wire(wire)
        {
                ptrFuns[0]= &DS1307::getSeconds;
                ptrFuns[1]= &DS1307::getMinutes;
                ptrFuns[2]= &DS1307::getDay;        
                ptrFuns[3]= &DS1307::getMonth;
        }
    
        ~DS1307(){
          
        }

        void getDateTime(String& datetime);
        void convertToDateTime(time_t in_timestamp,String& out_datetime);
        bool begin();

        bool isRunning(void);
        void startClock(void);
        void stopClock();

        void setHourMode(uint8_t h_mode);
        uint8_t getHourMode();

        void setMeridiem(uint8_t meridiem);
        uint8_t getMeridiem();

        void setSeconds(uint8_t second);
        void setMinutes(uint8_t minute);
        void setHours(uint8_t hour);
        void setDay(uint8_t day);
        void setWeek(uint8_t week);
        void setMonth(uint8_t month);
        void setYear(uint16_t year);

        void setDate(uint8_t day, uint8_t month, uint16_t year);
        void setTime(uint8_t hour, uint8_t minute, uint8_t second);

        bool setDateTime(const char* date,const  char* time);

        uint8_t getSeconds();
        uint8_t getMinutes();
        uint8_t getHours();
        uint8_t getDay();
        uint8_t getWeek();
        uint8_t getMonth();
        uint16_t getYear();

        void setEpoch(time_t epoch);
        time_t getEpoch();

        void setOutPin(uint8_t mode);
        bool isOutPinEnabled();
        bool isSqweEnabled();

        
  
protected:
        uint8_t bin2bcd (uint8_t val);
        uint8_t bcd2bin (uint8_t val);    
private:
        TwoWire& _wire;
        std::array<callbackFun, 4> ptrFuns;
};

class NVRAM
{
    public:
        NVRAM(TwoWire& wire):_wire(wire){

        }
        ~NVRAM(){
          
        }
        bool begin();
        uint8_t read(uint8_t address);
        void write(uint8_t address, uint8_t value);
        void read(uint8_t address, uint8_t *buf, uint8_t size);
        void write(uint8_t address, uint8_t *buf, uint8_t size);
        uint8_t length = 56;
    private:
     uint8_t             _sda;
        uint8_t             _scl;
        TwoWire& _wire;
};

/* Instantiate class

static DS3231 RTC;
static DS1307 RTC;
static NVRAM NVRAM;
static PCF8563 RTC;

*/

#endif   /* RTC_H */
