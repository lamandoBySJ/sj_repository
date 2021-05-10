#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include "arduino.h"
#include "app/OLEDScreen/OLEDScreen.h"
#include "platform/Callback.h"
#include <stddef.h>
#include <mutex>
#include <thread>
#include <stdarg.h>
//#define NDEBUG
#include "heltec.h"
namespace platform_debug
{
struct Builder {
        
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
    PlatformDebug()=default;
    ~PlatformDebug()=delete;

    using  FunPtrHardwareSerialPrintln = size_t(HardwareSerial::*)(const String&) ;
    using  FunPtrHardwareSerialPrintf = size_t(HardwareSerial::*)(const char*, ...);
    
    template <typename T,
    typename std::enable_if_t<std::is_same<T,OLEDScreen<12>>::value ||
     std::is_same<T,OLEDScreen<12>&>::value ,int> = 0 >
    static void SFINAE_init(T&& t)
    {
        Serial.println("T is OLEDScreen");
        if( std::is_lvalue_reference<decltype(t)>::value){
        //if( std::is_lvalue_reference<decltype(std::forward<T>(t))>::value){
            Serial.println("OLEDScreen--------------->LLLLLLLLLLLLLL");
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,&std::remove_reference_t<T>::println));
            PlatformDebug::_platformDebug->_onPrintLogoCallbacks.push_back(Callback<void()>(&t,&std::remove_reference_t<T>::logo));
           // PlatformDebug::_platformDebug->_onPrintfCallbacks.push_back(Callback<size_t(const char*, ...)>(&t,&std::remove_reference_t<T>::printf));
        }else{
            Serial.println("OLEDScreen--------------->RRRRRRRRRRRRRR");
            static OLEDScreen<12>* oled_ = new OLEDScreen<12>(std::forward<decltype(t)>(t));
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(oled_,&std::remove_reference_t<T>::println));
            PlatformDebug::_platformDebug->_onPrintLogoCallbacks.push_back(Callback<void()>(oled_,&std::remove_reference_t<T>::logo));
           // PlatformDebug::_platformDebug->_onPrintfCallbacks.push_back(Callback<size_t(const char*, ...)>(oled_,&std::remove_reference_t<T>::printf));
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
            //PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&HardwareSerial::println));
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
        }else{
            //Serial.println("HardwareSerial--------------->RRRRRRRRRRRRRR");
           // static HardwareSerial* serial_=new HardwareSerial(std::forward<decltype(t)>(t));
           // PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
           // PlatformDebug::_platformDebug->_onPrintfCallbacks.push_back( Callback<size_t(const char*,...)>(&t,(FunPtrHardwareSerialPrintf)&(std::remove_reference<decltype(t)>::type::printf)));
        }
    }

    template <class T,class...Args>
    static void init(T&& t,Args&&...args)
    {   //:_objs(std::forward<ArgTs>(args)...)
        #if !defined(NDEBUG)
        if(PlatformDebug::_finished){
              return;
        }else if(PlatformDebug::_platformDebug==nullptr){
            PlatformDebug::_platformDebug = new PlatformDebug(); 
        }
        SFINAE_init(std::forward<T>(t));
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
protected:  
 
    void print(const String& data) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data);
        }
        #endif
    }
    void print(const std::string& data) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
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
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
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
    static void inline init(){
      #if !defined(NDEBUG)
      PlatformDebug::_finished=true;
      Serial.println(">>>>>>>>>>>>>>>> Terminal init done <<<<<<<<<<<<<<<<<<<<<<<");
    
      #endif
    }
private:
    static PlatformDebug* _platformDebug;
    std::vector<Callback<size_t(const String& )>> _onPrintlnCallbacks;
    std::vector<Callback<void()>> _onPrintLogoCallbacks;
    static std::mutex _mtx;
    static bool _finished;
    static OLEDScreen<12>* _oled;
};

struct DeviceInfo
{
   static String BoardID;
   static String Family;
};

typedef struct {
    uint32_t counter=0;   
    String log;
} mail_trace_t;

class TracePrinter
{
public:
    explicit TracePrinter():_thread(osPriorityNormal,1024*6)
    {
        //platform_debug::PlatformDebug::println("TracePrinter");
    }
    TracePrinter&  operator=(const TracePrinter& other)=delete;
    TracePrinter&  operator=(TracePrinter&& other)=delete;
    
    static inline void  startup(){
        #if !defined(NDEBUG)
        if(TracePrinter::_tracePrinter==nullptr){
            _tracePrinter=new TracePrinter();
           _tracePrinter->_thread.start(callback(TracePrinter::_tracePrinter,&TracePrinter::run_trace_back));
           // _tracePrinter->_thread = std::thread(&TracePrinter::run_trace_back,TracePrinter::_tracePrinter);
        }else{
            return;
        }
        
        platform_debug::PlatformDebug::println(" ************ STLB ************ ");
        #endif
    }
 
   void run_trace_back(){
        #if !defined(NDEBUG)
        while(true){
           
            osEvent evt=  TracePrinter::_tracePrinter->_mail_box.get();
            if (evt.status == osEventMail) {
                mail_trace_t *mail = (mail_trace_t *)evt.value.p;
                 platform_debug::PlatformDebug::println(mail->log);
                 TracePrinter::_tracePrinter->_mail_box.free(mail); 
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
        printTrace(temp);
        #endif
    }
    
    static inline void printTrace(const String& e)
    {
        std::unique_lock<std::mutex>  lck(_mtx, std::defer_lock);
        lck.lock();
        mail_trace_t *mail =  TracePrinter::_tracePrinter->_mail_box.alloc();
        mail->log = e;
        TracePrinter::_tracePrinter->_mail_box.put(mail);
    }
 private:   
   

    #if !defined(NDEBUG)
    static TracePrinter* _tracePrinter;
    static std::mutex _mtx;
    rtos::Mail<mail_trace_t, 64> _mail_box;
    Thread _thread; 
    #endif
};

} // namespace platform_debug


#endif
