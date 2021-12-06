#include "app/RTC/DS3231MBuilder.h"

void DS3231MBuilder::power_on()
{

}
bool DS3231MBuilder::init(uint8_t h_mode)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    if(DS3231M_Class::isStopped()){
        while (!DS3231M_Class::begin())  // Initialize RTC communications
        {
            Serial.println(F("Unable to find DS3231MM. Checking again in 3s."));
            delay(3000);
        } 
                            // of loop until device is located
        DS3231M_Class::pinSquareWave();  // Make INT/SQW pin toggle at 1Hz
        Serial.println(F("DS3231M initialized."));
        DS3231M_Class::adjust();  // Set to library compile Date/Time
        Serial.print(F("DS3231M chip temperature is "));
        Serial.print(DS3231M_Class::temperature() / 100.0, 1);  // Value is in 100ths of a degree
        Serial.println(
            "\xC2\xB0"
            "C");
        Serial.println(F("\nEnter the following serial command:"));
        Serial.println(F("SETDATE yyyy-mm-dd hh:mm:ss"));
    }else{
        DS3231M_Class::init();
    }
return true;
}
bool DS3231MBuilder::setDateTime(const char* date,const  char* time)
{
std::lock_guard<rtos::Mutex> lck(_mtx);
    adjust(DateTime(__DATE__,__TIME__));

   return true;
}
time_t DS3231MBuilder::epoch()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    uint8_t day, month, hour, minute, second;
    struct tm tm_zone;
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	uint16_t year;
    DateTime&& now = DS3231M_Class::now();  // get the current time from device

    year=now.year();
    month=now.month();
    day =now.day();
    hour=now.hour();
    minute=now.minute();
    second=now.second();
      // Use sprintf() to pretty print the date/time with leading zeros
     /* char output_buffer[SPRINTF_BUFFER_SIZE];  ///< Temporary buffer for sprintf()
      sprintf(output_buffer, "%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(),
              now.hour(), now.minute(), now.second());
      Serial.println(output_buffer);
      secs = now.second();  // Set the counter variable
    */
  
    tm_zone.tm_sec = second;
	tm_zone.tm_min = minute;
    tm_zone.tm_hour = hour;
	tm_zone.tm_wday = now.dayOfTheWeek()-1;
	tm_zone.tm_mday = day;
	tm_zone.tm_mon = month-1;
	tm_zone.tm_year= year -1900; //ds1307 从2000年开始加

    time_t epoch = Convert::ToEpoch(&tm_zone);
    Serial.printf("year:%d\n",year);
    Serial.printf("month:%d\n",month);
    Serial.printf("day:%d\n",day);
    Serial.printf("hour:%d\n",hour);
    Serial.printf("minute:%d\n",minute);
    Serial.printf("second:%d\n",second);
    return epoch;
}

void DS3231MBuilder::setEpoch(time_t epoch)
{
    // of if the seconds
}