#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include "arduino.h"
#include "app/OLEDScreen/OLEDScreen.h"
#include "platform/Callback.h"
#include <stddef.h>
#include <mutex>
//#define NDEBUG

namespace platform_debug
{

struct beacon_properties{
    static String path;
};
/*
struct Builder {
        
};*/
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
            //Serial.println("OLEDScreen--------------->LLLLLLLLLLLLLL");
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,&std::remove_reference_t<T>::println));
            PlatformDebug::_platformDebug->_onPrintLogoCallbacks.push_back(Callback<void()>(&t,&std::remove_reference_t<T>::logo));
           // PlatformDebug::_platformDebug->_onPrintfCallbacks.push_back(Callback<size_t(const char*, ...)>(&t,&std::remove_reference_t<T>::printf));
        }else{
            //Serial.println("OLEDScreen--------------->RRRRRRRRRRRRRR");
            static OLEDScreen<12>* oled_ =new OLEDScreen<12>(std::forward<decltype(t)>(t));
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
            //Arduino Platform is defined
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
        #endif
    }
 
    static inline size_t printf(const char *format, ...)
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
            for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
                v.call(temp);
            }
            va_end(arg);
            return 0;
        };
        if(len >= sizeof(loc_buf)){
            temp = (char*) malloc(len+1);
            if(temp == NULL) {
                for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
                    v.call(temp);
                }
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
    /*
    static inline void println(const char* data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data);
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }*/
    
    static inline void println(std::string& data) 
    {
        #if !defined(NDEBUG)
         std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data.c_str());
        }
        #endif
    }

    static inline void println(const String& data) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data);
        }
        #endif
    }
    
protected:  
    static void inline init(){
      #if !defined(NDEBUG)
      PlatformDebug::_finished=true;
      Serial.println(">>>>>>>>>>>>>>>> Terminal init done <<<<<<<<<<<<<<<<<<<<<<<");
    
      #endif
    }
private:
    static PlatformDebug* _platformDebug;
    //std::vector<Callback<size_t(const char *)>> _onPrintfCallbacks;
    std::vector<Callback<size_t(const String& )>> _onPrintlnCallbacks;
    std::vector<Callback<void()>> _onPrintLogoCallbacks;
    static std::mutex _mtx;
    static bool _finished;
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
    TracePrinter(){
       
    }

    void startup(){
        
        #if !defined(NDEBUG)
        if(_tracePrinter==NULL){
            _tracePrinter=new TracePrinter();
            _thread.start(callback( _tracePrinter,&TracePrinter::run_debug_trace));
        } 
         #endif
    }
    void run_debug_trace(){

        #if !defined(NDEBUG)
        while(true){
            osEvent evt= _mail_box.get();
            if (evt.status == osEventMail) {
                mail_trace_t *mail = (mail_trace_t *)evt.value.p;
                 platform_debug::PlatformDebug::println(mail->log);
                _tracePrinter->_mail_box.free(mail); 
            }
        }
        //vTaskDelete(NULL);
        #endif
    }
    static inline  void printTrace(const char* e)
    {
        #if !defined(NDEBUG)
        TracePrinter::printTrace(String(e));
        #endif
    }

    static inline void printTrace(const String& e)
    {
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        mail_trace_t *mail = _tracePrinter->_mail_box.alloc();
        mail->log =DeviceInfo::BoardID+String(":")+ String(e);
        _tracePrinter->_mail_box.put(mail);
        #endif
    }

private:
    
    #if !defined(NDEBUG)
    static TracePrinter* _tracePrinter;
    rtos::Mail<mail_trace_t, 64> _mail_box;
    Thread _thread;
    static std::mutex _mtx;
    #endif
};
//std::vector<Callback<void(const char*)>>  PlatformDebug::_debug;

} // namespace platform_debug


#endif
