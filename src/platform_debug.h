#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include "arduino.h"
#include "app/OLEDScreen/OLEDScreen.h"
#include "platform/Callback.h"
#include <stddef.h>
#include <mutex>
#include <thread>
#include <stdarg.h>
#include "heltec.h"
#include "BH1749NUC_REG/bh1749nuc_reg.h"
#include <map>
//git clone -b 分支名 网址.git 
//git clone -b lesson-2 https://github.com/hemiahwu/vue-basic-playlist.git

void platform_debug_init(bool SerialEnabled=false,bool OLEDEnabled=false);
struct osStatusDictionary
{
   static String& getExceptionName(osStatus_t osStatus,const char* threadName,const char* thisThreadName=nullptr){
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

struct osStatusException:public std::exception
{
    osStatusException(osStatus_t osStatus,const char* threadName=nullptr)
    {
        this->osStatus  = osStatus;
        this->threadName = threadName;
    }
    String status;
    const char* threadName;
    osStatus_t osStatus;
    const char* what() const throw(){
        return osStatusDictionary::getExceptionName(osStatus,threadName,ThisThread::get_name()).c_str() ;
    }
};

//#define NDEBUG 
namespace platform_debug
{
struct mail_control_t{
  uint32_t counter=0; 
  uint32_t id=0;   
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

class Platform
{
public:
    static DeviceInfo deviceInfo;
    Platform(){

    }
};

//template <typename Signature>
//class PlatformDebug;
class PlatformDebug
{
public:
    PlatformDebug()
    { 

    }
    ~PlatformDebug(){
        Serial.println("~platform_debug::PlatformDebug::pause():~PlatformDebug");
        platform_debug::PlatformDebug::pause();
    }

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
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,&std::remove_reference_t<T>::println));
            PlatformDebug::_platformDebug->_onPrintLogoCallbacks.push_back(Callback<void()>(&t,&std::remove_reference_t<T>::logo));
           // PlatformDebug::_platformDebug._onPrintfCallbacks.push_back(Callback<size_t(const char*, ...)>(&t,&std::remove_reference_t<T>::printf));
        }else{
            Serial.println("OLEDScreen--------------->RRRRRRRRRRRRRR");
            static OLEDScreen<12>* oled_ = new OLEDScreen<12>(std::forward<decltype(t)>(t));
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(oled_,&std::remove_reference_t<T>::println));
            PlatformDebug::_platformDebug->_onPrintLogoCallbacks.push_back(Callback<void()>(oled_,&std::remove_reference_t<T>::logo));
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
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
        }else{
            //Serial.println("HardwareSerial--------------->RRRRRRRRRRRRRR");
           // static HardwareSerial* serial_=new HardwareSerial(std::forward<decltype(t)>(t));
           // PlatformDebug::_platformDebug._onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
           // PlatformDebug::_platformDebug._onPrintfCallbacks.push_back( Callback<size_t(const char*,...)>(&t,(FunPtrHardwareSerialPrintf)&(std::remove_reference<decltype(t)>::type::printf)));
        }
    }

    static void  init(){
      Serial.println(">>>>>>>>>>>>>>>> Terminal init done <<<<<<<<<<<<<<<<<<<<<<<");
    }
    template <class T,class...Args>
    static void init(T&& t,Args&&...args)
    { 
        #if !defined(NDEBUG)
        if(PlatformDebug::_platformDebug == nullptr){
              PlatformDebug::_platformDebug = new PlatformDebug();
        }
        SFINAE_init(std::forward<decltype(t)>(t));
        init(std::forward<Args>(args)...);
        #endif
    }

    static inline void printLogo() 
    {
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto v:PlatformDebug::_platformDebug->_onPrintLogoCallbacks){
            v.call();
        }
        lck.unlock();
        #endif
    }
 
    static inline size_t printf(const char *format, ...)
    {   
        #if !defined(NDEBUG)
       PlatformDebug::_platformDebug->print(format);
        #endif
        return 0;
    }
    static inline void println(std::string& data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->print(data);
        #endif
    }

    static inline void println(const String& data)  
    {   
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->print(data);
        #endif
    }
    
    static inline void pause(){
        while(1){  Serial.println("pause");ThisThread::sleep_for(Kernel::Clock::duration_seconds(10));};
    }
//protected:  
 
    void print(const String& data) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:_onPrintlnCallbacks){
            v.call(data);
        }
        #endif
    }
    void print(const std::string& data) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:_onPrintlnCallbacks){
            v.call(data.c_str());
        }
        #endif
    }

    size_t print(const char* format,...) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
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
    
    static  PlatformDebug* volatile _platformDebug;
//private:
   
    std::vector<Callback<size_t(const String& )>> _onPrintlnCallbacks;
    std::vector<Callback<void()>> _onPrintLogoCallbacks;
    static std::mutex _mtx;

};

typedef struct {
    uint32_t counter=0;   
    String log;
} mail_trace_t;

class TracePrinter
{
public:
    TracePrinter():_thread(osPriorityNormal,1024*6)
    {
        #if !defined(NDEBUG)
        _tracePrinter = this;
        #endif
    }
    ~TracePrinter()
    {
        Serial.println("~platform_debug::PlatformDebug::pause():~TracePrinter");
        platform_debug::PlatformDebug::pause();
    }
    TracePrinter&  operator=(const TracePrinter& other)=delete;
    TracePrinter&  operator=(TracePrinter&& other)=delete;
    
    void  startup(){
        #if !defined(NDEBUG)
        _thread.start(callback(this,&TracePrinter::run_trace_back));
        #endif
    }
 
   void run_trace_back(){
        while(true){
             osEvent evt=  _mail_box.get();
            if (evt.status == osEventMail) {
                mail_trace_t *mail = (mail_trace_t *)evt.value.p;
                 platform_debug::PlatformDebug::println(mail->log);
                 TracePrinter::_tracePrinter->_mail_box.free(mail); 
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
        TracePrinter::_tracePrinter->println(temp);
        #endif
    }
    static inline void printTrace(const String& e)
    {
        #if !defined(NDEBUG)
       TracePrinter::_tracePrinter->println(e);
       #endif
    }
    static inline void print(const char* e)
    {
        #if !defined(NDEBUG)
       TracePrinter::_tracePrinter->println(e);
       #endif
    }
private:
    void println(const String& e);

    #if !defined(NDEBUG)
    rtos::Thread _thread;
    rtos::Mail<mail_trace_t, 16>  _mail_box;
    rtos::Mutex _lck_mtx;
    static TracePrinter* volatile  _tracePrinter;
    #endif
};

} // namespace platform_debug



#endif
