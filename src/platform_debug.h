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
//git clone -b 分支名 网址.git 
//git clone -b lesson-2 https://github.com/hemiahwu/vue-basic-playlist.git

//#define NDEBUG 
namespace platform_debug
{
     
struct web_properties
{
    static String ap_ssid;
    static String ap_pass;
    static String http_user;
    static String http_pass;
    static String server_upload_uri;
};
struct rgb_properties
{
    static String path;
    static uint16_t r_offset;
    static uint16_t g_offset;
    static uint16_t b_offset;
};
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
struct user_properties{
static String path ;
static String ssid ;
static String pass ;
static String host ;
static int    port ;
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
    ~PlatformDebug()=default;

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
            PlatformDebug::_platformDebug._onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,&std::remove_reference_t<T>::println));
            PlatformDebug::_platformDebug._onPrintLogoCallbacks.push_back(Callback<void()>(&t,&std::remove_reference_t<T>::logo));
           // PlatformDebug::_platformDebug._onPrintfCallbacks.push_back(Callback<size_t(const char*, ...)>(&t,&std::remove_reference_t<T>::printf));
        }else{
            Serial.println("OLEDScreen--------------->RRRRRRRRRRRRRR");
            static OLEDScreen<12>* oled_ = new OLEDScreen<12>(std::forward<decltype(t)>(t));
            PlatformDebug::_platformDebug._onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(oled_,&std::remove_reference_t<T>::println));
            PlatformDebug::_platformDebug._onPrintLogoCallbacks.push_back(Callback<void()>(oled_,&std::remove_reference_t<T>::logo));
           // PlatformDebug::_platformDebug._onPrintfCallbacks.push_back(Callback<size_t(const char*, ...)>(oled_,&std::remove_reference_t<T>::printf));
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
            PlatformDebug::_platformDebug._onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
        }else{
            //Serial.println("HardwareSerial--------------->RRRRRRRRRRRRRR");
           // static HardwareSerial* serial_=new HardwareSerial(std::forward<decltype(t)>(t));
           // PlatformDebug::_platformDebug._onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
           // PlatformDebug::_platformDebug._onPrintfCallbacks.push_back( Callback<size_t(const char*,...)>(&t,(FunPtrHardwareSerialPrintf)&(std::remove_reference<decltype(t)>::type::printf)));
        }
    }

    template <class T,class...Args>
    static void init(T&& t,Args&&...args)
    {   //:_objs(std::forward<ArgTs>(args)...)
        #if !defined(NDEBUG)
        if(PlatformDebug::_finished){
              return;
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
        for(auto v:PlatformDebug::_platformDebug._onPrintLogoCallbacks){
            v.call();
        }
        lck.unlock();
        #endif
    }
 
    static inline size_t printf(const char *format, ...)
    {   
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug.print(format);
        #endif
        return 0;
    }
    static inline void println(std::string& data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug.print(data);
        #endif
    }

    static inline void println(const String& data) 
    {   
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug.print(data);
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
        for(auto& v:PlatformDebug::_platformDebug._onPrintlnCallbacks){
            v.call(data);
        }
        #endif
    }
    void print(const std::string& data) 
    {   
        #if !defined(NDEBUG)
        std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
        lck.lock();
        for(auto& v:PlatformDebug::_platformDebug._onPrintlnCallbacks){
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
        for(auto& v:PlatformDebug::_platformDebug._onPrintlnCallbacks){
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
    static PlatformDebug _platformDebug;
private:
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
    TracePrinter()
    {
        #if !defined(NDEBUG)
        _thread=new Thread(osPriorityNormal,1024*2);
        #endif
    }
    TracePrinter&  operator=(const TracePrinter& other)=delete;
    TracePrinter&  operator=(TracePrinter&& other)=delete;
    
    static inline void  startup(){
        #if !defined(NDEBUG)
            TracePrinter::_tracePrinter._thread->start(callback(&TracePrinter::_tracePrinter,&TracePrinter::run_trace_back));
        #endif
    }
 
   void run_trace_back(){
       Serial.println("run_trace_back");
        while(true){
            osEvent evt=  TracePrinter::_tracePrinter._mail_box.get();
            Serial.println("evt.status:"+String((int)evt.status,DEC));
            if (evt.status == osEventMail) {
                mail_trace_t *mail = (mail_trace_t *)evt.value.p;
                 platform_debug::PlatformDebug::println(mail->log);
                 TracePrinter::_tracePrinter._mail_box.free(mail); 
            }
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
        TracePrinter::_tracePrinter.println(temp);
        #endif
    }
    static inline void printTrace(const String& e)
    {
        #if !defined(NDEBUG)
       TracePrinter::_tracePrinter.println(e);
       #endif
    }

    void println(const String& e){
        #if !defined(NDEBUG)
        mail_trace_t *mail =  TracePrinter::_tracePrinter._mail_box.alloc();
        if(mail){
           mail->log = e;
            TracePrinter::_tracePrinter._mail_box.put(mail);
        }  
        #endif
    }
   static  TracePrinter _tracePrinter;
 private:   
    
    #if !defined(NDEBUG)
    Thread* _thread; 
    rtos::Mail<mail_trace_t, 16> _mail_box;
    #endif
};

} // namespace platform_debug


#endif
