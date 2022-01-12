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
    // 记录error级别的信息  
    template<typename T,typename U>
    static void error(T&& function,U&& line)
    {
       // #if !defined(NDEBUG)
        getInstance()._error(std::forward<String>(function),std::forward<int32_t>(line));
       // #endif
    }

    static void cleanup_errors()
    {
       // #if !defined(NDEBUG)
        getInstance().clear_errors();
       // #endif
    }
    static Logger& getInstance(){
        static Logger logger;
        return logger;
    }

    void error_log_get(JsonArray &data){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        for(auto& v:_error_logger){
           data.add(v.first+",line:"+String(v.second,DEC));
        }
    }
    void error_functions_get(String &data){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        data="{\"error_functions\":\"";
        for(auto& v:_error_logger){
           data+=v.first+",line:"+String(v.second,DEC) +",";
        }
        data+="\"}";
    }
    size_t get_error_count(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        return _error_logger.size();
    }
protected:
  
    void _error(const String& function,long line){
        std::lock_guard<rtos::Mutex> lck(getInstance()._mtx);
        _error_logger[function]=line;
    }
    void clear_errors(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        _error_logger.clear();
    }
private:  
    rtos::Mutex _mtx;
   // rtos::MemoryPool<logger_t,3> _pool;
   // std::vector<String> _debug_logger;
   // std::vector<String> _info_logger;
    std::map<String,int32_t> _error_logger;
};