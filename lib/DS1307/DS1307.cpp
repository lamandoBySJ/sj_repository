
#include "DS1307.h"
#include <time.h>

bool DS1307::begin()
{
  bool success=true;
    _wire.beginTransmission(0x68);
    success = (_wire.endTransmission() == 0 ? true : false);
    setOutPin(SQW001Hz);
	return success;
}    

bool DS1307::isRunning(void)
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);
  _wire.endTransmission();
       
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t data = _wire.read();
  bool flag = bitRead(data, 7);
	return (!flag);
}

void DS1307::startClock(void)
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t data = _wire.read();
  bitClear(data, 7);
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);  // Seconds Register
  _wire.write(data);
  _wire.endTransmission();
}

void DS1307::stopClock(void)
{ 
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t data = _wire.read();
  bitSet(data, 7);
  
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);  // Seconds Register
  _wire.write(data);
  _wire.endTransmission();
}

/*-----------------------------------------------------------
get & set HourMode
-----------------------------------------------------------*/

void DS1307::setHourMode(uint8_t h_mode)
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x02);  // Hour Register
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t data = _wire.read();
  bitWrite(data, 6, h_mode);
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x02);  // Hour Register
  _wire.write(data);
  _wire.endTransmission();
}

uint8_t DS1307::getHourMode()
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x02);
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t data = _wire.read();
  bool h_mode = bitRead(data, 6);
	return (h_mode);
}

/*-----------------------------------------------------------
-----------------------------------------------------------*/

void DS1307::setMeridiem(uint8_t meridiem)
{
  if (getHourMode() == CLOCK_H12) {
    _wire.beginTransmission(DS1307_ADDR);
    _wire.write(0x02);  // Hour Register
    _wire.endTransmission();
    _wire.requestFrom(DS1307_ADDR, 1);
    uint8_t data = _wire.read();
    bitWrite(data, 5, meridiem);
    
    _wire.beginTransmission(DS1307_ADDR);
    _wire.write(0x02);  // Hour Register
    _wire.write(data);
    _wire.endTransmission();
  } 
}

uint8_t DS1307::getMeridiem()
{

  if (getHourMode() == CLOCK_H12)
  {
    _wire.beginTransmission(DS1307_ADDR);
    _wire.write(0x02);
    _wire.endTransmission();
    _wire.requestFrom(DS1307_ADDR, 1);
    uint8_t data= _wire.read();
    return bitRead(data, 5);
       
  }
  else
   return HOUR_24;  
}

/*-----------------------------------------------------------
get & set Second
-----------------------------------------------------------*/
uint8_t DS1307::getSeconds()
{

  uint8_t daybcd=0;
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t second= _wire.read();
  bitClear(second, 7); // Clearing CH Bit if Set.
  daybcd = (bcd2bin(second));
	return daybcd;
}

void DS1307::setSeconds(uint8_t second)
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);  // Second Register
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t ch_bit = _wire.read();
  ch_bit = ch_bit & 0x80;
  second = ch_bit | second;
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);  // Second Register
  _wire.write(bin2bcd(second));
  _wire.endTransmission(); 
}

/*-----------------------------------------------------------
getMinute
-----------------------------------------------------------*/
uint8_t DS1307::getMinutes()
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x01);  // Minute Register
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t minute = _wire.read();
  return (bcd2bin(minute));
}

void DS1307::setMinutes(uint8_t minute)
{
      _wire.beginTransmission(DS1307_ADDR);
      _wire.write(0x01);  // Minute Register
      _wire.write(bin2bcd(minute));
      _wire.endTransmission();
}

/*-----------------------------------------------------------
getHours
-----------------------------------------------------------*/
uint8_t DS1307::getHours()
{
	uint8_t hour=0;
	bool h_mode = getHourMode();
  uint8_t daybcd=0;
      _wire.beginTransmission(DS1307_ADDR);
      _wire.write(0x02);  // Hour Register
      _wire.endTransmission();
      _wire.requestFrom(DS1307_ADDR, 1);
      hour = _wire.read();
      if (h_mode == CLOCK_H24)
      {
        daybcd = (bcd2bin(hour));
      }
      else if (h_mode == CLOCK_H12)
      {
        bitClear(hour, 5);
        bitClear(hour, 6);
        daybcd = (bcd2bin(hour));
      }
	 return daybcd;
}
#define UNUSED(expr) do { (void)(expr); } while (0)
void  DS1307::setHours(uint8_t hour)
{
	//bool meridiem;
 // UNUSED(meridiem);
  bool 	h_mode = getHourMode();
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x02);  // Hour Register
  if (h_mode == CLOCK_H24){
    _wire.write(bin2bcd(hour));
  }else if (h_mode == CLOCK_H12){
    if (hour > 12){
          hour = hour % 12;
          hour = bin2bcd(hour);
          bitSet(hour, 6);
          bitSet(hour, 5);
          _wire.write(hour);
    }
    else{
          hour = bin2bcd(hour);
          bitSet(hour, 6);
          bitClear(hour, 5);
          _wire.write(hour);
    }
  }
  _wire.endTransmission();
}

/*-----------------------------------------------------------
getWeek
-----------------------------------------------------------*/
uint8_t DS1307::getWeek()
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x03);  // Week Register
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t week = _wire.read();
	return week;
}

void DS1307::setWeek(uint8_t week)
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x03);  // Minute Register
  _wire.write(week);
  _wire.endTransmission();
}

/*-----------------------------------------------------------
getDay
-----------------------------------------------------------*/
uint8_t DS1307::getDay()
{
	_wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x04);  // Day Register
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t day = _wire.read();
  return (bcd2bin(day));
}

void DS1307::setDay(uint8_t day)
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x04);  // Day Register
  _wire.write(bin2bcd(day));
  _wire.endTransmission();  
}

/*-----------------------------------------------------------
getMonth()
-----------------------------------------------------------*/
uint8_t DS1307::getMonth()
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x05);  // Month Register
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint8_t month= _wire.read();
	return (bcd2bin(month));
}
/*-----------------------------------------------------------
setMonth()
-----------------------------------------------------------*/

void DS1307::setMonth(uint8_t month)
{
      _wire.beginTransmission(DS1307_ADDR);
      _wire.write(0x05);  // Month Register
      _wire.write(bin2bcd(month));
      _wire.endTransmission();
}

/*-----------------------------------------------------------
getYear (Completed)
-----------------------------------------------------------*/
uint16_t DS1307::getYear()
{
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x06);  // Year Register
  _wire.endTransmission();
  _wire.requestFrom(DS1307_ADDR, 1);
  uint16_t year = _wire.read();      
	return (bcd2bin(year) + 2000);
}

void DS1307::setYear(uint16_t year)
{
  uint16_t year_dummy = year % 100; //Converting to 2 Digit
  _wire.beginTransmission(DS1307_ADDR);  /* Writing 2 Digit year to Year Register(0x06) */
  _wire.write(0x06);  // Year Register to write year
  _wire.write(bin2bcd(year_dummy));
  _wire.endTransmission();
}

/*-----------------------------------------------------------
setTime
-----------------------------------------------------------*/

void DS1307::setTime(uint8_t hour, uint8_t minute, uint8_t second)
{
	bool h_mode = getHourMode();
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x00);
  _wire.write(bin2bcd(second));
  _wire.write(bin2bcd(minute));
       
  if (h_mode == CLOCK_H24){
    _wire.write(bin2bcd(hour));
  }
  else if (h_mode == CLOCK_H12){
    if (hour > 12){
      hour = hour % 12;
      hour = bin2bcd(hour);
      bitSet(hour, 6);
      bitSet(hour, 5);
      _wire.write(hour);
    }
    else{
      hour = bin2bcd(hour);
      bitSet(hour, 6);
      bitClear(hour, 5);
      _wire.write(hour);
    }
  }
  _wire.endTransmission(); 
}

/*-----------------------------------------------------------
setDate
-----------------------------------------------------------*/
void DS1307::setDate(uint8_t day, uint8_t month, uint16_t year)
{
  uint16_t	year_dummy = year % 100; //Converting to 2 Digit
  _wire.beginTransmission(DS1307_ADDR);
  _wire.write(0x04);
  _wire.write(bin2bcd(day));
  _wire.write(bin2bcd(month));
  _wire.write(bin2bcd(year_dummy));
  _wire.endTransmission();
}
/*-----------------------------------------------------------
setDateTime()
Taken from https://github.com/adafruit/RTClib/
-----------------------------------------------------------*/

bool DS1307::setDateTime(const char* date,const char* time)
{
	uint8_t day, month, hour, minute, second;
  struct tm tm_zone;
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	uint16_t year = atoi(date + 9);
	setYear(year);
	// Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	switch (date[0]) {
	case 'J': month = (date[1] == 'a') ? 1 : ((date[2] == 'n') ? 6 : 7); break;
	case 'F': month = 2; break;
	case 'A': month = date[2] == 'r' ? 4 : 8; break;
	case 'M': month = date[2] == 'r' ? 3 : 5; break;
	case 'S': month = 9; break;
	case 'O': month = 10; break;
	case 'N': month = 11; break;
	case 'D': month = 12; break;
  default:month=0;break;
	}
	setMonth(month);
	day = atoi(date + 4);
	setDay(day);
	hour = atoi(time);
	setHours(hour);
	minute = atoi(time + 3);
	setMinutes(minute);
	second = atoi(time + 6);
	setSeconds(second);

 
  tm_zone.tm_sec = second;
	tm_zone.tm_min = minute;
  tm_zone.tm_hour = hour;
	tm_zone.tm_wday = getWeek()-1;
	tm_zone.tm_mday = day;
	tm_zone.tm_mon = month-1;
	tm_zone.tm_year =2000 + year -1900; //ds1307 从2000年开始加

  time_t epoch = Convert::ToEpoch(&tm_zone);
  setEpoch(epoch-28800);

  if(year!= getYear()%100){
      return false;
  }
  if(hour!= getHours()+8){
      return false;
  }

  std::array<uint16_t,4> assert_date;
  assert_date[0]= second;
  assert_date[1]= minute;
  assert_date[2]= day;
  assert_date[3]= month;
  auto func=ptrFuns.begin();
  bool ok =  std::all_of(assert_date.begin(),assert_date.end(),[this,&func](uint16_t val)->bool{
      return val == (this->**func++)();
  });
  return ok;

}
/*-----------------------------------------------------------
setEpoch()
-----------------------------------------------------------*/

void DS1307::setEpoch(time_t epoch)
{
	struct tm *epoch_tm = gmtime(&epoch);
	setSeconds(epoch_tm->tm_sec); //0x00 - Seconds
	setMinutes(epoch_tm->tm_min);
	setHours(epoch_tm->tm_hour);
	setWeek(epoch_tm->tm_wday+1);
	setDay(epoch_tm->tm_mday);
	setMonth(epoch_tm->tm_mon+1);
	setYear(epoch_tm->tm_year + 1900);
	//_wire.endTransmission();
}

/*-----------------------------------------------------------
getEpoch()
-----------------------------------------------------------*/
time_t DS1307::getEpoch()
{
	struct tm epoch_tm;
	epoch_tm.tm_sec = getSeconds();
	epoch_tm.tm_min = getMinutes();
	epoch_tm.tm_hour = getHours();
	epoch_tm.tm_wday = getWeek()-1;
	epoch_tm.tm_mday = getDay();
	epoch_tm.tm_mon = getMonth()-1;
	epoch_tm.tm_year = getYear()-1900;
	return  mktime(&epoch_tm);
}


void DS1307::getDateTime(String& datetime)
{
  time_t epochTimeZone = getEpoch();
  Convert::ToDateTime(epochTimeZone,datetime);
}

/*placement new benefit：
1）在已分配好的内存上进行对象的构建，构建速度快。
2）已分配好的内存可以反复利用，有效的避免内存碎片问题。
*/
/* NVRAM Functions */

bool NVRAM::begin()
{
  bool success=false;
  
  _wire.begin(this->_sda, this->_scl,100000);
	_wire.beginTransmission(0x68);
  success = (_wire.endTransmission() == 0 ? true : false);
  
	return success;
}


uint8_t NVRAM::read(uint8_t address)
{
   
	uint8_t data = 0x00;
	address = (address % length) + 0x08;
	_wire.beginTransmission(DS1307_ADDR);
	_wire.write(address);
	_wire.endTransmission();
	_wire.requestFrom(DS1307_ADDR, 1);
	data = _wire.read();
   
	return (data);
}
void NVRAM::write(uint8_t address, uint8_t data)
{
   
	address = (address % length) + 0x08;
	_wire.beginTransmission(DS1307_ADDR);
	_wire.write(address);
	_wire.write(data);
	_wire.endTransmission();
   
}

void NVRAM::read(uint8_t address, uint8_t* buf, uint8_t size)
{
   
	int addrByte = 0x08 + address;
	_wire.beginTransmission(DS1307_ADDR);
	_wire.write(addrByte);
	_wire.endTransmission();
	_wire.requestFrom(DS1307_ADDR, 9);
	for (uint8_t pos = 0; pos < size; ++pos)
	{
		buf[pos] = _wire.read();
	}
   
}

void NVRAM::write(uint8_t address, uint8_t* buf, uint8_t size)
{
   
	int addrByte = 0x08 + address;
	_wire.beginTransmission(DS1307_ADDR);
	_wire.write(addrByte);
	for (uint8_t pos = 0; pos < size; ++pos)
	{
		_wire.write(buf[pos]);
	}
	_wire.endTransmission();
   
}

/* SQW/OUT pin functions */

void DS1307::setOutPin(uint8_t mode)
{
   
	_wire.beginTransmission(DS1307_ADDR);
	_wire.write(0x07);
	switch (mode) {
	case HIGH:
		_wire.write(B10000000);
		break;
	case LOW:
		_wire.write(B00000000);
		break;
	case SQW001Hz:
		_wire.write(B00010000);
		break;
	case SQW04kHz:
		_wire.write(B00010001);
		break;
	case SQW08kHz:
		_wire.write(B00010010);
		break;
	case SQW32kHz:
		_wire.write(B00010011);
		break;
	}
	_wire.endTransmission();
   
}

bool DS1307::isOutPinEnabled()
{
   
	uint8_t data;
	_wire.beginTransmission(DS1307_ADDR);
	_wire.write(0x07);
	_wire.endTransmission();
	_wire.requestFrom(DS1307_ADDR, 1);
	data = _wire.read();
	data = bitRead(data, 7);
   
	return (data);
}

bool DS1307::isSqweEnabled()
{
   
	uint8_t data;
	_wire.beginTransmission(DS1307_ADDR);
	_wire.write(0x07);
	_wire.endTransmission();
	_wire.requestFrom(DS1307_ADDR, 1);
	data = _wire.read();
	data = bitRead(data, 4);
   
	return (data);
}

/* Helpers */

uint8_t DS1307::bcd2bin(uint8_t val)
{
	return val - 6 * (val >> 4);
}
uint8_t DS1307::bin2bcd(uint8_t val)
{
	return val + 6 * (val / 10);
}
