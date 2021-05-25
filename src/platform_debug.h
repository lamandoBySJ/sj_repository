#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H


#include "app/OLEDScreen/OLEDScreen.h"
#include "BH1749NUC_REG/bh1749nuc_reg.h"
#include "arduino.h"
#include "platform/Callback.h"
#include <stddef.h>
#include <mutex>
#include <thread>
#include <stdarg.h>
#include "heltec.h"
#include <map>
#include "platform/mbed.h"
#include "rtos/Thread.h"
#include "rtos/Mail.h"
#include "app/OLEDScreen/OLEDScreen.h"
//#define NDEBUG 
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

}

struct RGB
{
    RGB(){
        R.u16bit = 0;
        G.u16bit = 0;
        B.u16bit = 0;
        IR.u16bit = 0;
    }
    reg_uint16_t R;
    reg_uint16_t G;
    reg_uint16_t B;
    reg_uint16_t IR;
    uint32_t h;
    uint32_t s;
    uint32_t l;
};

enum class MeasEventType : char{
        EventSystemMeasure = 0,
        EventServerMeasure,
        EventWebAppOffset,
        EventWebAppMeasure
};
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
    struct RGBProperties
    {   
        RGBProperties(){
            path ="/als_constant";
            r_offset = 0;
            g_offset = 0;
            b_offset = 0;
        }
        String path;
        uint16_t r_offset;
        uint16_t g_offset;
        uint16_t b_offset;

        RGBProperties& operator=(const RGBProperties& properties){
           this->path =  properties.path;
           this->r_offset = properties.r_offset;
           this->g_offset = properties.g_offset;
           this->b_offset = properties.b_offset;
           return *this;
        }
    };

class Platform
{
public:

    Platform()=delete;
    ~Platform()=delete;
    static DeviceInfo& getDeviceInfo(){
        static DeviceInfo deviceInfo;
        return deviceInfo;
    }
    static WebProperties& getWebProperties()
    {
        static WebProperties webProperties;
        return webProperties;
    }
    static UserProperties& getUserProperties()
    {
        static  UserProperties  userProperties;
        return userProperties;
    }

    static RGBProperties& getRGBProperties()
    {
        static  RGBProperties  RGBproperties;
        return RGBproperties;
    }

   
};
struct IPSProtocol
{
    IPSProtocol(){
        technology="LoRa";
        family="k49a";
        gateway="9F8C";
        collector="9F8C";
        mode = "learn";
    }
    String technology;
    String family;
    String gateway;
    String collector;
    String mode;
};





//template <typename Signature>
//class PlatformDebug;
class PlatformDebug
{
public:
    using  FunPtrHardwareSerialPrintln = size_t(HardwareSerial::*)(const String&) ;
    using  FunPtrHardwareSerialPrintf = size_t(HardwareSerial::*)(const char*, ...);
    
    template <typename T,
    typename std::enable_if_t<std::is_same<T,OLEDScreen<12>>::value ||
     std::is_same<T,OLEDScreen<12>&>::value ,int> = 0 >
    static void SFINAE_init(T&& t)
    {
        Serial.println("T is OLEDScreen");
        if( std::is_lvalue_reference<decltype(t)>::value){
            Serial.println("OLEDScreen--------------->LLLLLLLLLLLLLL");
            getInstance()->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,&std::remove_reference_t<T>::println));
            getInstance()->_onPrintLogoCallbacks.push_back(Callback<void()>(&t,&std::remove_reference_t<T>::logo));
           // getInstance()._onPrintfCallbacks.push_back(Callback<size_t(const char*, ...)>(&t,&std::remove_reference_t<T>::printf));
        }else{
            Serial.println("OLEDScreen--------------->RRRRRRRRRRRRRR");
            static OLEDScreen<12>* oled_ = new OLEDScreen<12>(std::forward<decltype(t)>(t));
            getInstance()->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(oled_,&std::remove_reference_t<T>::println));
            getInstance()->_onPrintLogoCallbacks.push_back(Callback<void()>(oled_,&std::remove_reference_t<T>::logo));
        }
    }

    template <typename T,
    typename std::enable_if_t<std::is_same<T,HardwareSerial>::value ||
       std::is_same<T,HardwareSerial&>::value ,int> = 0>
    static void SFINAE_init(T&& t)
    {
        Serial.println("T is HardwareSerial");
        if( std::is_lvalue_reference<decltype(std::forward<T>(t))>::value){
            //Serial.println("HardwareSerial--------------->LLLLLLLLLLLLLL");
            //PlatformDebug::_platformDebug._onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&HardwareSerial::println));
            getInstance()->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
        }else{
            //Serial.println("HardwareSerial--------------->RRRRRRRRRRRRRR");
           // static HardwareSerial* serial_=new HardwareSerial(std::forward<decltype(t)>(t));
           //getInstance()._onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
           // getInstance()._onPrintfCallbacks.push_back( Callback<size_t(const char*,...)>(&t,(FunPtrHardwareSerialPrintf)&(std::remove_reference<decltype(t)>::type::printf)));
        }
    }
    
    void  init(){
      Serial.println(">>>>>>>>>>>>>>>> Terminal init done <<<<<<<<<<<<<<<<<<<<<<<");
      
    }

    template <class T,class...Args>
    void init(T&& t,Args&&...args)
    { 
        #if !defined(NDEBUG)
        SFINAE_init(std::forward<decltype(t)>(t));
        init(std::forward<Args>(args)...);
        #endif
    }


    static PlatformDebug* getInstance()
    { 
        static PlatformDebug* platformDebug=new PlatformDebug();
        return platformDebug;
    }

    static inline void printLogo() 
    {
        #if !defined(NDEBUG)
        for(auto v:getInstance()->_onPrintLogoCallbacks){
            v.call();
        }
        #endif
    }
 
    static inline size_t printf(const char *format, ...)
    {   
        #if !defined(NDEBUG)
        getInstance()->print(format);
        #endif
        return 0;
    }
    static inline void println(std::string& data) 
    {
        #if !defined(NDEBUG)
        getInstance()->print(data);
        #endif
    }

    static inline void println(const String& data)  
    {   
        #if !defined(NDEBUG)
        getInstance()->print(data);
        #endif
    }
    
    static inline void pause(){
        while(1){  Serial.println("pause");ThisThread::sleep_for(Kernel::Clock::duration_seconds(10));};
    }
protected:  
 
    void print(const String& data) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:_onPrintlnCallbacks){
            v.call(data);
        }
        #endif
    }
    void print(const std::string& data) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:_onPrintlnCallbacks){
            v.call(data.c_str());
        }
        #endif
    }

    size_t print(const char* format,...) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<rtos::Mutex> lck(_mtx, std::defer_lock);
        lck.lock();

        char loc_buf[64];
        char * temp = loc_buf;
        va_list arg;
        va_list copy;
        va_start(arg, format);
        va_copy(copy, arg);
        int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
        va_end(copy);
        if(len < 0) {
            va_end(arg);
            return 0;
        }
        if(len >= sizeof(loc_buf)){
            temp = (char*) malloc(len+1);
            if(temp == NULL) {
                va_end(arg);
                return 0;
            }
            len = vsnprintf(temp, len+1, format, arg);
        }
        va_end(arg);
       // len = write((uint8_t*)temp, len);
        for(auto& v:_onPrintlnCallbacks){
            v.call(temp);
        }
        if(temp != loc_buf){
            free(temp);
        }
        return len;
        #else 
            return 0;
        #endif
    }
    
    static  PlatformDebug*  _platformDebug;

    ~PlatformDebug(){
        Serial.println("~platform_debug::PlatformDebug::pause():~PlatformDebug");
        PlatformDebug::pause();
    }
private:
    PlatformDebug(){}
    PlatformDebug(const PlatformDebug&)=delete;
    PlatformDebug& operator=(const PlatformDebug&)=delete;
    std::vector<Callback<size_t(const String& )>> _onPrintlnCallbacks;
    std::vector<Callback<void()>> _onPrintLogoCallbacks;
    rtos::Mutex _mtx;
};

struct [[gnu::may_alias]] mail_trace_t{
    uint32_t id;
    String log;
};

class TracePrinter
{
public:
    TracePrinter():_thread(osPriorityNormal,1024*6)
    {

    }
     ~TracePrinter()
    {
        Serial.println("~platform_debug::PlatformDebug::pause():~TracePrinter");
    }
   
    void  startup(){
        #if !defined(NDEBUG)
        Serial.printf("this:%p\n",this);
        Serial.printf("_thread:%p\n",&_thread);
        Serial.printf("_mail_box:%p\n", &_mail_box);
        Serial.printf("_mtx;:%p\n", &_mtx);  
        if(_thread.get_state()!=Thread::Running){
            _thread.start(callback(this,&TracePrinter::run_trace_back));
        }
        Serial.println("tracePrinter is Running:"+String(__FILE__));
        #endif
    }
 
   void run_trace_back(){
        while(true){
             osEvent evt=  _mail_box.get();
            if (evt.status == osEventMail) {
                mail_trace_t *mail = (mail_trace_t *)evt.value.p;
                 PlatformDebug::println(mail->log);
                 TracePrinter::getInstance()->_mail_box.free(mail); 
            }
            //std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
 
    static inline void printf(const char* format,...)
    {
        #if !defined(NDEBUG)
       
        char loc_buf[64];
        char * temp = loc_buf;
        va_list arg;
        va_list copy;

        va_start(arg, format);
        va_copy(copy, arg);
        int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
        va_end(copy);
        if(len < 0) {
            va_end(arg);
            return;
        };
        if(len >= sizeof(loc_buf)){
            temp = (char*) malloc(len+1);
            if(temp == NULL) {
                va_end(arg);
                return;
            }
            len = vsnprintf(temp, len+1, format, arg);
        }
        va_end(arg);
       // len = write((uint8_t*)temp, len);
        TracePrinter::getInstance()->println(temp);
        #endif
    }
    static inline void printTrace(const String& e)
    {
        #if !defined(NDEBUG)
       TracePrinter::getInstance()->println(e);
       #endif
    }
    static inline void print(const char* e)
    {
        #if !defined(NDEBUG)
       TracePrinter::getInstance()->println(e);
       #endif
    }
    static TracePrinter* getInstance(){
       static TracePrinter *tracePrinter = new TracePrinter();
        return tracePrinter;
    }
    
    TracePrinter&  operator=(const TracePrinter& other)=delete;
    TracePrinter&  operator=(TracePrinter&& other)=delete;
    
private:
    void println(const String& e);
    #if !defined(NDEBUG)
    rtos::Mail<mail_trace_t, 8>  _mail_box;
    rtos::Thread _thread;
    rtos::Mutex _mtx;
    #endif
};
#endif
