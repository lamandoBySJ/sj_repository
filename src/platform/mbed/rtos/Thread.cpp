#include "rtos/Thread.h"

using namespace rtos;

Thread::~Thread()
{

}

void Thread::_thunk(void *thread_ptr)
{
   //(static_cast< mbed::Callback<void()> * > (thread_ptr))->call();
   (static_cast< Thread * > (thread_ptr))->_task();
}

void Thread::constructor(const char * pcName,
										const uint32_t usStackDepth,
										UBaseType_t uxPriority,
										TaskHandle_t* const pvCreatedTask,
										const BaseType_t xCoreID)
{
   // (TaskFunction_t)(&Thread::_thunk);
   // _pvTaskCode= pvTaskCode;
    _pcName = pcName;
    _usStackDepth=usStackDepth;
   // _pvParameters=pvParameters;
    _uxPriority= uxPriority;
    _pvCreatedTask = pvCreatedTask;
    _xCoreID =   xCoreID;
}

osStatus Thread::start(mbed::Callback<void()> task)
{
    this->_task=callback(task);
    //Thread::_thunk(this);
   // xTaskCreatePinnedToCore( (TaskFunction_t)&Thread::_thunk, 
    //_pcName, _usStackDepth, _pvParameters, _uxPriority, this, _xCoreID );
    xTaskCreatePinnedToCore( (TaskFunction_t)&Thread::_thunk, 
    _pcName, _usStackDepth, this, _uxPriority, _pvCreatedTask, _xCoreID );
    return osOK;
}
