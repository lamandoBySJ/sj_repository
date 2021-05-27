#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H


#include "app/OLEDScreen/OLEDScreen.h"
#include "BH1749NUC_REG/bh1749nuc_reg.h"
#include "arduino.h"
#include <stddef.h>
#include <mutex>
#include <thread>
#include <stdarg.h>
#include "heltec.h"
#include <map>
#include "platform/mbed.h"
#include "LEDIndicator.h"

#define NDEBUG 
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
namespace platform
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
} //namespace platform

class Platform
{
public:

    Platform()=delete;
    ~Platform()=delete;
    static DeviceInfo& get_device_Info(){
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

    static RGBProperties& get_rgb_properties()
    {
        static  RGBProperties  RGBproperties;
        return RGBproperties;
    }

     static String& get_version()
    {
        static  RGBProperties  RGBproperties;
        return platform::version(__DATE__,__TIME__);
    }
   
};

class PlatformDebug
{
public:
    using  FunPtrHardwareSerialPrintln = size_t(HardwareSerial::*)(const String&) ;
    using  FunPtrHardwareSerialPrintf = size_t(HardwareSerial::*)(const char*, ...);

    static PlatformDebug* getInstance()
    { 
        static PlatformDebug* platformDebug=new PlatformDebug();
        return platformDebug;
    }

    template <typename T,
    typename std::enable_if_t<std::is_same<T,OLEDScreen<12>>::value ||
     std::is_same<T,OLEDScreen<12>&>::value ,int> = 0 >
    static void SFINAE_init(T&& t)
    {   
        #if !defined(NDEBUG)
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
        #endif
    }

    template <typename T,
    typename std::enable_if_t<std::is_same<T,HardwareSerial>::value ||
       std::is_same<T,HardwareSerial&>::value ,int> = 0>
    static void SFINAE_init(T&& t)
    {  
        #if !defined(NDEBUG)
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
        #endif
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
        std::lock_guard<rtos::Mutex> lck(_mtx);
        for(auto& v:_onPrintlnCallbacks){
            v.call(data);
        }
        #endif
    }
    void print(const std::string& data) 
    {   
        #if !defined(NDEBUG)
        std::lock_guard<rtos::Mutex> lck(_mtx);
        for(auto& v:_onPrintlnCallbacks){
            v.call(data.c_str());
        }
        #endif
    }

    size_t print(const char* format,...) 
    {   
        #if !defined(NDEBUG)
        std::lock_guard<rtos::Mutex> lck(_mtx);

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

    ~PlatformDebug(){
        Serial.println("~platform_debug::PlatformDebug::pause():~PlatformDebug");
        PlatformDebug::pause();
    }
    
private:
    #if !defined(NDEBUG)
    
    PlatformDebug(){}
    PlatformDebug(const PlatformDebug&)=delete;
    PlatformDebug& operator=(const PlatformDebug&)=delete;
    std::vector<Callback<size_t(const String& )>> _onPrintlnCallbacks;
    std::vector<Callback<void()>> _onPrintLogoCallbacks;
    rtos::Mutex _mtx;
    #endif
};

struct [[gnu::may_alias]] mail_trace_t{
    uint32_t id;
    String log;
};

class TracePrinter
{
public:
   
    TracePrinter()
    {
        #if !defined(NDEBUG)
        _thread = new Thread(osPriorityNormal,1024*6);
        #endif
    }
     ~TracePrinter()
    {
        Serial.println("~platform_debug::PlatformDebug::pause():~TracePrinter");
    }
   
    void  startup(){
        #if !defined(NDEBUG)
        if(_thread->get_state()!=Thread::Running){
            _thread->start(callback(this,&TracePrinter::run_trace_back));
        }
        #endif
    }
 
   void run_trace_back(){
        #if !defined(NDEBUG)
        while(true){
             osEvent evt=  _mail_box.get();
            if (evt.status == osEventMail) {
                mail_trace_t *mail = (mail_trace_t *)evt.value.p;
                 PlatformDebug::println(mail->log);
                 TracePrinter::getInstance()->_mail_box.free(mail); 
            }
        }
        #endif
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
 
    
    static inline void init(){
        #if !defined(NDEBUG)
        getInstance()->startup();
        #endif
    }
 
    TracePrinter&  operator=(const TracePrinter& other)=delete;
    TracePrinter&  operator=(TracePrinter&& other)=delete;
    
private:
    #if !defined(NDEBUG)
    void println(const String& e){
        #if !defined(NDEBUG)
        std::lock_guard<rtos::Mutex> lck(_mtx);
        mail_trace_t *mail = _mail_box.alloc();
        mail->log = e;
        _mail_box.put(mail);
        #endif
    }
    static TracePrinter* getInstance(){
       static TracePrinter *tracePrinter = new TracePrinter();
        return tracePrinter;
    }
    
    rtos::Mail<mail_trace_t, 8>  _mail_box;
    rtos::Thread *_thread;
    rtos::Mutex _mtx;
    #endif
};

#endif
