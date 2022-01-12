#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include "app/OLEDScreen/OLEDScreen.h"
#include "MemoryManager.h"
//#define NDEBUG 


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
            Serial.println("OLEDScreen--------------->LLLLLLLLLLLLLL");
            getInstance()->_onPrintlnCallbacks.push_back( mbed::Callback<size_t(const String&)>(&t,&std::remove_reference_t<T>::println));  
        }else{
            Serial.println("OLEDScreen--------------->RRRRRRRRRRRRRR");
            static OLEDScreen<12>* oled_ = new OLEDScreen<12>(std::forward<decltype(t)>(t));
            getInstance()->_onPrintlnCallbacks.push_back( mbed::Callback<size_t(const String&)>(oled_,&std::remove_reference_t<T>::println));
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
    template <typename... T>
    static inline size_t printf(const char* format,T&&... args)
    {
        #if !defined(NDEBUG)
        getInstance()->_printf(format,std::forward<T>(args)...);
        #endif
        return 0;
    }

    static inline void println(const String& data)  
    {   
        #if !defined(NDEBUG)
        getInstance()->printf("%s\n",data.c_str());
        #endif
    }
    
    static inline void pause(){
        #if !defined(NDEBUG)
        while(1){ getInstance()->printf("\npause\n");ThisThread::sleep_for(Kernel::Clock::duration_seconds(30));};
        #endif
    }
protected:  


    size_t _printf(const char* format,...) 
    {   
        #if !defined(NDEBUG)
        std::lock_guard<rtos::Mutex> lck(_mtx);

        va_list arg;
        va_start(arg, format);

        int len = vsnprintf(_loc_buf, _loc_buf_len, format, arg);
        if(len < 0) {
            va_end(arg);
            return len;
        };

        if(len > _loc_buf_len){
            int buf_len=(len/_loc_buf_len+1)*_loc_buf_len;
            char* temp = nullptr;
            temp = _loc_buf == _buffer ? (char*)realloc(temp,buf_len+1):(char*)realloc(_loc_buf,buf_len+1);
            
            if(temp) {
                _loc_buf_len=buf_len;
                _loc_buf    =temp;
            }else{
                va_end(arg);
                return len;
            }
            
            len = vsnprintf(_loc_buf, _loc_buf_len, format, arg);
            if(len < 0) {
                va_end(arg);
                return len;
            }else{
                
            }
            _loc_buf[_loc_buf_len]=0;
        }
            
        for(auto& v:_onPrintlnCallbacks){
            v.call(_loc_buf);
        }
        va_end(arg);
        return len;
        #else 
        return 0;
        #endif
    }

    ~PlatformDebug(){
        if(_loc_buf!=_buffer){
             free(_loc_buf);
        }
        Serial.printf("~platform_debug\n");
    }
    
private:
    #if !defined(NDEBUG)
    char* _loc_buf;
    int _loc_buf_len;
    char _buffer[256];
    PlatformDebug(){ 
        _loc_buf_len=sizeof(_buffer)-1;
        _loc_buf=_buffer;
    }

    PlatformDebug(const PlatformDebug&)=delete;
    PlatformDebug& operator=(const PlatformDebug&)=delete;

    std::vector<mbed::Callback<size_t(const String&)>> _onPrintlnCallbacks;
    std::vector<mbed::Callback<void()>> _onPrintLogoCallbacks;
    rtos::Mutex _mtx;
    #endif
};
/*
struct [[gnu::may_alias]] mail_trace_t{
    uint32_t id;
    String log;
};
*/
struct mail_trace_t :public os_memory_t{
   /* template <typename... T>
    const char* copy(const char* format,T&&... args){
       return os_allocator_t::copy(&log,format,std::forward<T>(args)...);
    }*/
     const char* log;
};
class TracePrinter
{
public:
     ~TracePrinter()
    {
        free(_loc_buf);
        _thread->terminate();
        delete _thread;
        Serial.println("~TracePrinter");
    }
   
    void  init(){
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
                 PlatformDebug::printf(mail->log);
                _mail_box.free(mail); 
            }
        }
        #endif
    }
    template <typename... T>
    static inline void printf(const char* format,T&&... args)
    {
        #if !defined(NDEBUG)
        TracePrinter::getInstance()->_printf(format,std::forward<T>(args)...);
        #endif
    }
    static inline void printTrace(const String& e)
    {
        #if !defined(NDEBUG)
       TracePrinter::getInstance()->_printf(e.c_str());
       #endif
    }
    static inline void println(const String& e)
    {
        #if !defined(NDEBUG)
       TracePrinter::getInstance()->_printf(e.c_str());
       #endif
    }
    static inline void startup(){
        #if !defined(NDEBUG)
        TracePrinter::getInstance()->init();
        #endif
    }
 
    TracePrinter&  operator=(const TracePrinter& other)=delete;
    TracePrinter&  operator=(TracePrinter&& other)=delete;
    
private:
    #if !defined(NDEBUG)

    void _printf(const char* format,...)
    {
     
        #if !defined(NDEBUG)
        std::lock_guard<rtos::Mutex> lck(_mtx);
        va_list arg;
        va_start(arg, format);
        int len = vsnprintf(_loc_buf, _loc_buf_len, format, arg);
        if(len < 0) {
            va_end(arg);
            return;
        };

        if(len > _loc_buf_len){
            int buf_len=(len/_loc_buf_len+1)*_loc_buf_len;
            char* temp = nullptr;
            if(_loc_buf == _buffer){
                temp=(char*)realloc(temp,buf_len+1);
            }else{
                temp=(char*)realloc(_loc_buf,buf_len+1);
            }
            
            if(temp) {
                _loc_buf_len=buf_len;
                _loc_buf    =temp;
            }else{
                va_end(arg);
                return;
            }
            
            len = vsnprintf(_loc_buf, _loc_buf_len, format, arg);
            if(len < 0) {
                va_end(arg);
                return;
            };
            _loc_buf[_loc_buf_len]=0;
        }
        va_end(arg);
        mail_trace_t* mail = _mail_box.alloc();
        if(mail){
             mail->log = mail->copy(_loc_buf);
            _mail_box.put(mail);
        }
        #endif
    }
    static TracePrinter* getInstance(){
       static TracePrinter* tracePrinter = new TracePrinter();
        return tracePrinter;
    }
    TracePrinter()
    {
        #if !defined(NDEBUG)
        _loc_buf_len=sizeof(_buffer)-1;
        _loc_buf=_buffer;
        _thread = new Thread(osPriorityNormal,1024*6);
        #endif
    }
    char* _loc_buf;
    char _buffer[256];
    int _loc_buf_len;

    rtos::Mail<mail_trace_t, 8>  _mail_box;
    rtos::Thread* _thread;
    rtos::Mutex _mtx;
    #endif
};

#endif
