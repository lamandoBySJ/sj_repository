#include "LightSensor.h"

LightSensor::LightSensor(rtos::Mutex& mutex):_mutex(mutex)
{
    _rst = 0;
}

LightSensor::LightSensor(rtos::Mutex& mutex,uint8_t rst):_mutex(mutex)
{
    _rst = rst;
}

void LightSensor::startup(void *pvParameters)
 {
    if(pvParameters != NULL){
      digitalWrite(_rst,HIGH);
      ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
    }

    _mutex.lock();
     
    _mutex.unlock();
 }