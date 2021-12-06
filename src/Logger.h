#pragma once
#include "ArduinoJson.h"
#include "platform_debug.h"
typedef struct 
{
    uint32_t id;
    String log;
}logger_t;

class Logger
{
public:
    Logger(){

    }
     // 记录debug级别的信息
    static inline void debug(const String& function,int32_t line)
    { 
        //#if !defined(NDEBUG)
        getInstance().debug("debug_fun:"+function+",line:"+String(line,DEC));
       // #endif
    }
     static inline void debug(const String& namespace_,const String& func_name,int32_t line)
    { 
        //#if !defined(NDEBUG)
        getInstance().debug(namespace_+String("::")+func_name+",line:"+String(line,DEC));
       // #endif
    }
        // 记录info级别的信息
     static inline  void  info(const String& function,int32_t line)
    {
       // #if !defined(NDEBUG)
        getInstance().info("info_fun:"+function+",line:"+String(line,DEC));
       // #endif
    }
     static inline  void  info(const String& namespace_,const String& func_name,int32_t line)
    {
       // #if !defined(NDEBUG)
        getInstance().info(namespace_+String("::")+func_name+",line:"+String(line,DEC));
       // #endif
    }
        // 记录error级别的信息
    static inline  void error(const String& function,int32_t line)
    {
       // #if !defined(NDEBUG)
        getInstance().error(function+",line:"+String(line,DEC));
       // #endif
    }
    static inline  void error(const String& namespace_,const String& func_name,int32_t line)
    {
       // #if !defined(NDEBUG)
        getInstance().error(namespace_+String("::")+func_name+",line:"+String(line,DEC));
       // #endif
    }
    static Logger& getInstance(){
        static Logger logger;
        return logger;
    }

    void debug_log_get(JsonArray &data){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        for(auto& v:_debug_logger){
           data.add(v);
        }
    }
    void info_log_get(JsonArray &data){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        for(auto& v:_info_logger){
           data.add(v);
        }
    }
    void error_log_get(JsonArray &data){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        for(auto& v:_error_logger){
           data.add(v);
        }
    }

protected:
    void debug(const String& msg){
        std::lock_guard<rtos::Mutex> lck(getInstance()._mtx);
        _debug_logger.push_back(msg);
    }
    void info(const String& msg){
        std::lock_guard<rtos::Mutex> lck(getInstance()._mtx);
       _info_logger.push_back(msg);
    }
    void error(const String& msg){
        std::lock_guard<rtos::Mutex> lck(getInstance()._mtx);
        _error_logger.push_back(msg);
    }
private:  
    rtos::Mutex _mtx;
   // rtos::MemoryPool<logger_t,3> _pool;
    std::vector<String> _debug_logger;
    std::vector<String> _info_logger;
    std::vector<String> _error_logger;
   
};