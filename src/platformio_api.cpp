#include "platformio_api.h"

namespace platformio_api
{
static String&  version(const char* date,const char* time)
{
	uint8_t day, month, hour, minute, second;
	uint16_t year;
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	year = atoi(date + 9);
	//setYear(year);
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
	//setMonth(month);
	day = atoi(date + 4);
	//setDay(day);
	hour = atoi(time);
	//setHours(hour);
	minute = atoi(time + 3);
	//setMinutes(minute);
	second = atoi(time + 6);
	//setSeconds(second);
    static String version= 
    String(year,DEC)+String(".")+
    String(month,DEC)+String(".")+
    String(day,DEC)+String(".")+
    String(hour,DEC)+String(".")+
    String(minute,DEC)+String(".")+
    String(second,DEC);
    return version;
}

DeviceInfo& get_device_info(){
    static DeviceInfo deviceInfo;
    return deviceInfo;
}
WebProperties& get_web_properties()
{
    static WebProperties webProperties;
    return webProperties;
}
UserProperties& get_user_properties()
{
    static  UserProperties  userProperties;
    return userProperties;
}
String& get_version()
{
    return version(__DATE__,__TIME__);
}
}// namespace platformio_api

