#include "rtos/Mutex.h"
#include "FreeRTOS.h"

using namespace rtos;

Mutex::Mutex()
{
   _semaphore = xSemaphoreCreateMutex();
}
Mutex::~Mutex()
{
    
}
Mutex::Mutex(const char *name)
{

}

void Mutex::lock()
{
    while( xSemaphoreTake(_semaphore, ( TickType_t ) portMAX_DELAY) != pdTRUE ){
       // ThisThread::sleep_for(Kernel::Clock::duration_u32(100));
    }
}

bool Mutex::trylock()
{
    if( xSemaphoreTake(_semaphore, ( TickType_t ) 100) != pdTRUE ){
       return false;
    }
    return true;
}

bool Mutex::trylock_for(uint32_t millisec)
{
    if( xSemaphoreTake(_semaphore, ( TickType_t ) millisec) != pdTRUE ){
       return false;
    }
    return true;
}

bool Mutex::trylock_for(Kernel::Clock::duration_u32 rel_time)
{
    if( xSemaphoreTake(_semaphore, ( TickType_t ) rel_time.count()) != pdTRUE ){
       return false;
    }
    return true;
}

bool Mutex::trylock_until(uint64_t millisec)
{
    if( xSemaphoreTake(_semaphore, ( TickType_t ) millisec) != pdTRUE ){
       return false;
    }
    return true;
}

bool Mutex::trylock_until(Kernel::Clock::time_point abs_time)
{
    if( xSemaphoreTake(_semaphore, ( TickType_t )abs_time.time_since_epoch().count()) != pdTRUE ){
       return false;
    }
    return true;
}

void Mutex::unlock()
{
    xSemaphoreGive(_semaphore);
}

osThreadId_t Mutex::get_owner()
{
    return 0;
}