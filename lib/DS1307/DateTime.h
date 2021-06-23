/*
 * DateTime.cpp - DateTime Class for DateTime Operations
 * Created by Manjunath CV. May 29, 2020, 06:28 PM
 * Released into the public domain.
 */

#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <time.h>



class DateTime {
  public:
    DateTime();
    DateTime(uint16_t y, uint8_t mo, uint8_t d, uint8_t h, uint8_t mi, uint8_t s);
    DateTime(uint8_t hour, uint8_t min, uint8_t sec);
    uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t day;
        uint8_t week;
        uint8_t month;
    uint16_t year;      
};



#endif
