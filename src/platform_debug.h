#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include "arduino.h"
#include "app/OLEDScreen/OLEDScreen.h"
#include "platform/Callback.h"
#include <stddef.h>
//#define NDEBUG

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


    template <typename T,
    typename = std::enable_if_t<!std::is_lvalue_reference<T>::value>,
    typename = std::enable_if_t<std::is_same<T,TwoWire>::value,TwoWire > >
    static std::enable_if_t<std::is_same<T,TwoWire>::value,void> SFINAE_test(T&& t)
    {
        Serial.println("T is TwoWire");
    }

    template <typename T,
    typename = std::enable_if_t<!std::is_lvalue_reference<T>::value>,
    typename = std::enable_if_t<std::is_same<T,OLEDScreen<12>>::value,OLEDScreen<12> > >
    static std::enable_if_t<std::is_same<T,OLEDScreen<12>>::value,void> SFINAE_test(T&& t)
    {
        Serial.println("T is OLEDScreen");
        PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,&T::println));
       // PlatformDebug::_platformDebug->_onPrintCallbacks.push_back(Callback<size_t(const char*,...)>(const_cast<T*>(&t),&T::printf));
        PlatformDebug::_platformDebug->_onPrintLogoCallbacks.push_back(Callback<void()>(&t,&T::logo));
    }

   using  FunctionPtrHardwareSerial = size_t(HardwareSerial::*)(const String&) ;

    template <typename T,
    typename = std::enable_if_t<!std::is_lvalue_reference<T>::value>,
    typename = std::enable_if_t<std::is_same<T,HardwareSerial>::value,HardwareSerial > >
    static std::enable_if<std::is_same<T,HardwareSerial>::value,void> SFINAE_test(T&& t)
    {
        Serial.println("T is HardwareSerial");
        PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunctionPtrHardwareSerial)&T::println));
    }

    

    template <class T,class...Args>
    static void init(const T& t,const Args&...args)//:_objs(std::forward<ArgTs>(args)...)
    {
        #if !defined(NDEBUG)
        if(PlatformDebug::_finished){
              return;
        }else if(PlatformDebug::_platformDebug==nullptr){
            PlatformDebug::_platformDebug = new PlatformDebug(); 
        }
        SFINAE_test(std::forward<T>(const_cast<T&>(t)));
        init(std::forward<const Args>(args)...);
        #endif
    }
   static inline void printLogo() 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        for(auto v:PlatformDebug::_platformDebug->_onPrintLogoCallbacks){
            v.call();
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }
    /*
    static inline void print(const char* data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        for(auto& v:PlatformDebug::_platformDebug->_onPrintCallbacks){
            v.call(data);
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }*/
    static inline void print(const String& data) 
    {
        //PlatformDebug::print(data.c_str());
         #if !defined(NDEBUG)
       // PlatformDebug::_platformDebug->std_mutex.lock();
       // for(auto& v:PlatformDebug::_platformDebug->_onPrintCallbacks){
      //      v.call(data.c_str());
      //  }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }

    static inline size_t printf(const char *format, ...)
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
            return 0;
        };
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
       PlatformDebug::print(temp);
        if(temp != loc_buf){
            free(temp);
        }
        return len;
        #else 
            return 0;
        #endif
    }
  
    static inline void println(const char* data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data);
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }
    static inline void println(const std::string& data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data.c_str());
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }
    static inline void println(const String& data) 
    {   /*
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        for(auto v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data.c_str());
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif*/
        PlatformDebug::println(data.c_str());
    }
    
private:  
    static void inline init(){
      #if !defined(NDEBUG)
      PlatformDebug::_finished=true;
      Serial.println(">>>>>>>>>>>>>>>> Terminal init done <<<<<<<<<<<<<<<<<<<<<<<");
      #endif
    }
private:
    static PlatformDebug* _platformDebug;
    //std::vector<Callback<size_t(const char *, ...)>> _onPrintCallbacks;
    std::vector<Callback<size_t(const String& )>> _onPrintlnCallbacks;
    std::vector<Callback<void()>> _onPrintLogoCallbacks;
    static rtos::Mutex std_mutex;
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
        _tracePrinter->std_trace_mutex.lock();
        mail_trace_t *mail = _tracePrinter->_mail_box.alloc();
        mail->log =DeviceInfo::BoardID+String(":")+ String(e);
        _tracePrinter->_mail_box.put(mail) ;
        _tracePrinter->std_trace_mutex.unlock();
        #endif
    }

private:
    
    #if !defined(NDEBUG)
    static TracePrinter* _tracePrinter;
    rtos::Mail<mail_trace_t, 64> _mail_box;
    Thread _thread;
    static rtos::Mutex std_trace_mutex;
    #endif
};
//std::vector<Callback<void(const char*)>>  PlatformDebug::_debug;

} // namespace platform_debug


#endif
