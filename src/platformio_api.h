#ifndef PLATFORMIO_API_H
#define PLATFORMIO_API_H

#include "Arduino.h"
#include <map>
#include "platform/mbed.h"


namespace platformio_api
{
#ifndef PROJECT_STLB
struct I2C_BUS_NUM_RTC : public std::integral_constant<unsigned int, 1> {};
struct I2C_BUS_SCL_RTC : public std::integral_constant<unsigned int, 32> {};
struct I2C_BUS_SDA_RTC : public std::integral_constant<unsigned int, 33> {};
struct I2C_BUS_RST_RTC : public std::integral_constant<unsigned int, 0> {};

#else
struct I2C_BUS_NUM_RTC : public std::integral_constant<unsigned int, 0> {};
struct I2C_BUS_SCL_RTC : public std::integral_constant<unsigned int, 21> {};
struct I2C_BUS_SDA_RTC : public std::integral_constant<unsigned int, 22> {};
struct I2C_BUS_RST_RTC : public std::integral_constant<unsigned int, 13> {};

#endif

struct DeviceInfo
{   
    explicit  DeviceInfo(){
        BoardID="ABCD";
        Family="k49a";
    }

    DeviceInfo&  operator= (DeviceInfo& other)=delete;
    DeviceInfo&  operator= (DeviceInfo&& other)=delete;

    String BoardID;
    String Family;
};

struct  UserProperties{
    explicit UserProperties()
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
      explicit WebProperties(){
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

DeviceInfo& get_device_info();
WebProperties& get_web_properties();
UserProperties& get_user_properties();
String& get_version();

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
struct rtc_error:public std::exception
{
        rtc_error(const String& detail)
        {
            this->detail = detail.c_str();
        }
        const char* detail;
        const char* what() const throw(){
            return this->detail;
        }
};

}// namespace os

namespace i2c
{
struct pin_error:public std::exception
{
        pin_error(const String& detail)
        {
            this->detail = detail.c_str();
        }
        const char* detail;
        const char* what() const throw(){
            return this->detail;
        }
};

}//namespace i2c
#endif