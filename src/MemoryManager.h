#pragma once
#include "Arduino.h"
#include <map>

struct os_buffer_t
{
    os_buffer_t(){
        this->mem=buffer;
        this->size=sizeof(buffer)-1;
        len=0;
    }
    ~os_buffer_t(){
        if(mem!=buffer){
            free(mem);
        }
    }
   char buffer[256];
   char* mem;
   size_t len;
   size_t size;
};

template<class T>
class MemoryManager
{
public:
    ~MemoryManager(){}
static  os_buffer_t* os_memory(T* mem_ptr){
  return getInstance().os_buffer(std::forward<T*>(mem_ptr));
   //return getInstance().os_buffer(mem_ptr);
}
private:
MemoryManager()=default;
static MemoryManager<T>& getInstance(){
    static MemoryManager<T> manager;
    return manager;
}
os_buffer_t* os_buffer(T* mem_ptr){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    return &_map[mem_ptr];
}
std::map<T*,os_buffer_t> _map;
rtos::Mutex _mtx;
};

struct os_allocator_t
{
    os_allocator_t(){}
    virtual ~os_allocator_t(){}
    const char* copy(os_buffer_t* buffer,const char* format,...){
        va_list arg;
        va_start(arg, format);
        int len = vsnprintf(buffer->mem, buffer->size+1, format, arg);
        if(len < 0) {
            va_end(arg);
            return buffer->mem;
        };
        if(len > buffer->size){
            int mem_len = buffer->size*(len / buffer->size+1);
            char* temp = nullptr;
            temp = buffer->mem==buffer->buffer ? (char*)realloc(temp,mem_len+1):(char*)realloc(buffer->mem,mem_len+1);
           
            if(temp){
                buffer->size = mem_len;
                buffer->mem = temp;
                len = vsnprintf(buffer->mem, buffer->size+1, format, arg);
                if(len < 0) {
                    va_end(arg);
                    return buffer->mem;
                };
            } 
        }
        buffer->len=len;
        va_end(arg);
        return buffer->mem;
    }
};
struct os_memory_t : public os_allocator_t
{
    os_memory_t(){}
    virtual ~os_memory_t(){

    }
    template <typename... T>
    const char* copy(const char* format,T&&... args){
       return os_allocator_t::copy(MemoryManager<os_memory_t>::os_memory(this),format,std::forward<T>(args)...);
    }
    const char* memory(){
        return MemoryManager<os_memory_t>::os_memory(this)->mem;
    }
    const size_t size(){
        return MemoryManager<os_memory_t>::os_memory(this)->size;
    }
    const size_t length(){
        return MemoryManager<os_memory_t>::os_memory(this)->len;
    }
    //char* mem=nullptr;
    //uint32_t mem_len=0;
};