#ifndef PLATFORM_H
#define PLATFORM_H

#include "Arduino.h"
#include <map>
#include "platform/mbed.h"
#include "product/product_stlb.h"

namespace platformio
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

struct DeviceInfo
{   
    DeviceInfo(){
        BoardID="ABCD";
        Family="k49a";
    }
    String BoardID;
    String Family;
};

struct  UserProperties{
    UserProperties()
    {   
        path="/user_constant";
        ssid = "IoTwlan";
        pass = "mitac1993";
        host = "mslmqtt.mic.com.cn";
        port = 1883;
    }
    String  path ;
    String  ssid ;
    String  pass ;
    String  host ;
    int     port ;
    UserProperties& operator=(const UserProperties& properties){
        this->path = properties.path;
        this->ssid = properties.ssid;
        this->pass = properties.pass;
        this->host = properties.host;
        this->port = properties.port;
        return *this;
    }
};
struct WebProperties
{   
        WebProperties(){
            ap_ssid="STLB";
            ap_pass="Aa000000";
            http_user="admin";
            http_pass="admin";
            server_upload_url = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
        }
        String ap_ssid;
        String ap_pass;
        String http_user;
        String http_pass;
        String server_upload_url;

        WebProperties& operator=(const WebProperties& properties){
            this->ap_ssid = properties.ap_ssid;
            this->ap_pass = properties.ap_pass;
            this->http_user = properties.http_user;
            this->http_pass = properties.http_pass;
            this->server_upload_url = properties.server_upload_url;
            return *this;
        }
};
struct api
{

static DeviceInfo& get_device_info(){
        static DeviceInfo deviceInfo;
        return deviceInfo;
}
static WebProperties& get_web_properties()
{
        static WebProperties webProperties;
        return webProperties;
}
static UserProperties& get_user_properties()
{
        static  UserProperties  userProperties;
        return userProperties;
}
static String& get_version()
{
        return platformio::version(__DATE__,__TIME__);
}
};

}//namespace platformio

namespace os
{ 


struct osThread
{
    static String& get_error_reason(osStatus_t osStatus,const char* threadName,const char* thisThreadName=nullptr){
            static String e;
            static std::map<osStatus_t,String> _map;
            if(_map.size()==0){
                _map[osOK]="osOK";
                _map[osErrorTimeout]="osErrorTimeout";
                _map[osErrorParameter ]="osErrorParameter ";
                _map[osErrorResource]="osErrorResource";
                _map[osErrorNoMemory]="osErrorNoMemory";
                _map[osErrorISR]="osErrorISR";
                _map[osStatusReserved ]="osStatusReserved ";
            }
            e = "--- "+_map[osStatus]+"[ code:"+String((int)osStatus,DEC)+",Thread:"+String(threadName)+",ThisThread:"+String(thisThreadName)+" ]"+" ---";
            return e;
        }
 };

struct thread_error:public std::exception
{
        thread_error(osStatus_t osStatus,const char* threadName=nullptr)
        {
            this->osStatus  = osStatus;
            this->threadName = threadName;
        }
        String status;
        const char* threadName;
        osStatus_t osStatus;
        const char* what() const throw(){
            return osThread::get_error_reason(osStatus,threadName,ThisThread::get_name()).c_str() ;
        }
};
struct alloc_error:public std::exception
{
        alloc_error(const String& detail)
        {
            this->detail = detail.c_str();
        }
        const char* detail;
        const char* what() const throw(){
            return this->detail;
        }
};
}// namespace os
#endif