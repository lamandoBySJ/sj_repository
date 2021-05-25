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
#include <time.h>
#include <Wire.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <RTCBase.h>
/*
 * DateTime Class
 * */
 
#define DS1307_ADDR 0x68

class DS1307 //: public RTCBase
{
    public:
        DS1307() = delete;
        DS1307(TwoWire& wire,uint8_t  sda,uint8_t scl,uint32_t frequency=100000):
        _wire(wire),_sda(sda),_scl(scl),_frequency(frequency)
        {
            
        }
    
        ~DS1307(){
          
        }
        //void setDateTime(time_t timestamp);
       // time_t datetime(String& nowtime);
       // virtual time_t timestamp(String& nowtime) override;
        void getDateTime(String& datetime,bool duplicate=false);
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

        void setDateTime(const char* date,const  char* time);

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

        
        static time_t _epoch;
        
    private:
        TwoWire& _wire;
        uint8_t bin2bcd (uint8_t val);
        uint8_t bcd2bin (uint8_t val);
        uint8_t             _sda;
        uint8_t             _scl;
        uint32_t _frequency;
       
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
