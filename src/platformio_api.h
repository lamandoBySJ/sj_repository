#pragma once

#include "Arduino.h"
#include <map>
#include "platform/mbed.h"
#include "WiFi.h"
struct Device{
    Device(){
        _WiFiMacAddress = WiFi.macAddress();
        _WiFiMacAddress.replace(":","");
        unsigned int len= _WiFiMacAddress.length();
        for(char i=len;i>0;i--){
            _map[i]= _WiFiMacAddress.substring(len-i,len);
        }
    }
    static Device& getInstance(){
        static Device device;
        return device;
    }
    static String& ID(char digits=4){
      return getInstance()._map[digits];
    }
    static void dump(){
        for(auto& v:getInstance()._map){
            Serial.println(v.second);
        }
    }
    static String& WiFiMacAddress(){
        return getInstance()._WiFiMacAddress;
    }
    std::map<char,String> _map;
     String _WiFiMacAddress;
};
struct  User{
    User()
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
    User& operator=(const User& properties){
        this->path = properties.path;
        this->ssid = properties.ssid;
        this->pass = properties.pass;
        this->host = properties.host;
        this->port = properties.port;
        return *this;
    }
    static User& getProperties()
    {
        static  User userProperties;
        return userProperties;
    }
};

namespace platformio_api
{


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

//DeviceInfo& get_device_info();
WebProperties& get_web_properties();
//UserProperties& get_user_properties();
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

}// namespace os