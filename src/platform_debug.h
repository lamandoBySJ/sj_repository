#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include "platformio_api.h"
#include "app/OLEDScreen/OLEDScreen.h"
#include <platform/mbed.h>

#define NDEBUG 
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
    typename std::enable_if_t<std::is_same<std::remove_reference_t<T>,OLEDScreen<12>>::value,int> = 0 >
    static void SFINAE_init(T&& t)
    {   
        #if !defined(NDEBUG)
        Serial.println("T is OLEDScreen");
        if( std::is_lvalue_reference<decltype(t)>::value){
            getInstance()->_onPrintlnCallbacks.push_back(mbed:: Callback<size_t(const String&)>(&t,&std::remove_reference_t<T>::println));
            getInstance()->_onPrintLogoCallbacks.push_back(mbed::Callback<void()>(&t,&std::remove_reference_t<T>::logo));
           // getInstance()._onPrintfCallbacks.push_back(Callback<size_t(const char*, ...)>(&t,&std::remove_reference_t<T>::printf));
        }else{
            static OLEDScreen<12>* oled_ = new OLEDScreen<12>(std::forward<decltype(t)>(t));
            getInstance()->_onPrintlnCallbacks.push_back( mbed::Callback<size_t(const String&)>(oled_,&std::remove_reference_t<T>::println));
            getInstance()->_onPrintLogoCallbacks.push_back(mbed::Callback<void()>(oled_,&std::remove_reference_t<T>::logo));
        }
        #endif
    }

    template <typename T,
    typename std::enable_if_t<std::is_same<std::remove_reference_t<T>,HardwareSerial>::value,int> = 0>
    static void SFINAE_init(T&& t)
    {  
        #if !defined(NDEBUG)
        Serial.println("T is HardwareSerial");
        if( std::is_lvalue_reference<decltype(std::forward<T>(t))>::value){
            //Serial.println("HardwareSerial--------------->LLLLLLLLLLLLLL");
            //PlatformDebug::_platformDebug._onPrintlnCallbacks.push_back( Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&HardwareSerial::println));
            getInstance()->_onPrintlnCallbacks.push_back(mbed::Callback<size_t(const String&)>(&t,(FunPtrHardwareSerialPrintln)&std::remove_reference<decltype(t)>::type::println));
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
    std::vector<mbed::Callback<size_t(const String& )>> _onPrintlnCallbacks;
    std::vector<mbed::Callback<void()>> _onPrintLogoCallbacks;
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
            _thread->start(mbed::callback(this,&TracePrinter::run_trace_back));
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
        if(temp != loc_buf){
            free(temp);
        }
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
    //rtos::MemoryPool<char*,8> _buf;
    rtos::Mail<mail_trace_t, 8>  _mail_box;
    rtos::Thread *_thread;
    rtos::Mutex _mtx;
    #endif
};

#endif
