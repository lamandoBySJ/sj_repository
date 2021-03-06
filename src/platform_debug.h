#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include "arduino.h"
#include "app/OLEDScreen/OLEDScreen.h"
#include "platform/Callback.h"

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
    /*PlatformDebug(ArgTs...args)
    { 
        static int cnt = 0;
        static int cnt2 = sizeof...(ArgTs);
        Serial.println(cnt2);
        if(std::is_same<HardwareSerial,T>::value){
             Serial.println("::is_same<HardwareSerial");
        }else if(std::is_same<TwoWire,T>::value){
                Serial.println("::is_same<TwoWire");
        }else if(std::is_same<OLEDScreen<12>,T>::value){
                Serial.println("::is_same<OLEDScreen<12>");
        }else{
                Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
        }
        
        
        if(--cnt2>=0){
          //  _objs(std::forward<ArgTs>(args)...)
         
            PlatformDebug(std::forward<ArgTs>(args)...);
        }
    }
    */
    template <class U,class...Args>
    static void init(U&& u,Args&&...args)//:_objs(std::forward<ArgTs>(args)...)
    {
        #if !defined(NDEBUG)
        if(PlatformDebug::_finished){
              return;
        }else if(PlatformDebug::_platformDebug==nullptr){
            PlatformDebug::_platformDebug = new PlatformDebug(); 
        }

        if(std::is_same<HardwareSerial, U>::value){
            Serial.println("::is_same<HardwareSerial");
        }else if(std::is_same<TwoWire, U>::value){
            Serial.println("::is_same<TwoWire");
        }else if(std::is_same<OLEDScreen<12>, U>::value){
            Serial.println("::is_same<OLEDScreen<12>");
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back(Callback<void(const String&)>(&u,&U::println));
            PlatformDebug::_platformDebug->_onPrintCallbacks.push_back(Callback<void(const char*)>(&u,&U::printf));
            PlatformDebug::_platformDebug->_onPrintLogoCallbacks.push_back(Callback<void()>(&u,&U::logo));
        }else if(std::is_same< Print , U>::value){
            Serial.println("::is_same<Print>"); 
        }else{
            Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
        }
        //Callback<void(const char*)>(&u,&U::println);
        init(std::forward<Args>(args)...);
        #endif
    }
    template <class U,class...Args>
    static void init(U& u,Args&...args)//:_objs(std::forward<ArgTs>(args)...)
    {
        #if !defined(NDEBUG)
        if(PlatformDebug::_finished){
              return;
        }else if(PlatformDebug::_platformDebug==nullptr){
            PlatformDebug::_platformDebug = new PlatformDebug(); 
        }

        if(std::is_same<HardwareSerial, U>::value){
            Serial.println("::is_same<HardwareSerial");
        }else if(std::is_same<TwoWire, U>::value){
            Serial.println("::is_same<TwoWire");
        }else if(std::is_same<OLEDScreen<12>, U>::value){
            Serial.println("::is_same<OLEDScreen<12>");
            PlatformDebug::_platformDebug->_onPrintlnCallbacks.push_back(Callback<void(const String&)>(&u,&U::println));
            PlatformDebug::_platformDebug->_onPrintCallbacks.push_back(Callback<void(const char*)>(&u,&U::printf));
            PlatformDebug::_platformDebug->_onPrintLogoCallbacks.push_back(Callback<void()>(&u,&U::logo));
        }else if(std::is_same< Print , U>::value){
            Serial.println("::is_same<Print>"); 
        }else{
            Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
        }
        //Callback<void(const char*)>(&u,&U::println);
        init(std::forward<Args>(args)...);
        #endif
    }
    static void inline init(){
      #if !defined(NDEBUG)
      PlatformDebug::_finished=true;
      Serial.println("#################### INIT DONE ####################");
      #endif
    }
 

    
    static inline void printLogo() 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        //Serial.println(data);
        for(auto v:PlatformDebug::_platformDebug->_onPrintLogoCallbacks){
            v.call();
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }

    static inline void print(const char* data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        Serial.println(data);
        for(auto& v:PlatformDebug::_platformDebug->_onPrintCallbacks){
            v.call(data);
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }

    /*
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        Serial.println(data);
        for(auto v:PlatformDebug::_platformDebug->_onPrintCallbacks){
            v.call(data.c_str());
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif*/
    static inline void print(const String& data) 
    {
        PlatformDebug::print(data.c_str());
        Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>"+String(PlatformDebug::_platformDebug->_onPrintCallbacks.size(),DEC));

    }
    static inline size_t printf(const char *format, ...)
    {

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
       PlatformDebug::println(temp);
        if(temp != loc_buf){
            free(temp);
        }
        return len;
    }

    static inline void println(const char* data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        Serial.println(data);
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
        Serial.println(data.c_str());
        for(auto& v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data.c_str());
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }
    static inline void println(const String& data) 
    {
        /*
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        Serial.println(data);
        for(auto v:PlatformDebug::_platformDebug->_onPrintlnCallbacks){
            v.call(data.c_str());
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif*/
        PlatformDebug::println(data.c_str());
    }
     /*template <class U,class...ArgTs>
    void printArgs(U u,ArgTs...args) {
      // x.printf("x","x");
      u.println("************From T println**************");  
      if(std::is_same<HardwareSerial,T>::value){
             Serial.println("::is_same<HardwareSerial");
      }else if(std::is_same<TwoWire,T>::value){
             Serial.println("::is_same<TwoWire");
      }else if(std::is_same<OLEDScreen<12>,T>::value){
             Serial.println("::is_same<OLEDScreen<12>");
      }else{
             Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
      }

      print(std::forward<ArgTs>(args)...);
    }
  
    template <class U,class...ArgTs>
    void printArgs(U u,ArgTs...args) {
      // x.printf("x","x");
      t.println("************From T println**************");  
      if(std::is_same<HardwareSerial,T>::value){
             Serial.println("::is_same<HardwareSerial");
      }else if(std::is_same<TwoWire,T>::value){
             Serial.println("::is_same<TwoWire");
      }else if(std::is_same<OLEDScreen<12>,T>::value){
             Serial.println("::is_same<OLEDScreen<12>");
      }else{
             Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
      }

      print(std::forward<ArgTs>(args)...);
    } */

private:
    static PlatformDebug* _platformDebug;
    std::vector<Callback<void(const char*)>> _onPrintCallbacks;
    std::vector<Callback<void(const String& )>> _onPrintlnCallbacks;
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
