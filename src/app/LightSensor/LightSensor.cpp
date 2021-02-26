#include "LightSensor.h"

int32_t LightSensor::thunk_write_reg(void *thread_ptr)
{
  (static_cast< LightSensor * > (thread_ptr))->_task_write();
}

int32_t LightSensor::thunk_read_reg(void *thread_ptr)
{
  (static_cast< LightSensor * > (thread_ptr))->_task_read();
}

LightSensor::LightSensor(BH1749NUC& bh1749nuc,rtos::Mutex& mutex):_bh1749nuc(bh1749nuc),_mutex(mutex)
{
    _rst = 0;
}

LightSensor::LightSensor(BH1749NUC& bh1749nuc,rtos::Mutex& mutex,uint8_t rst):_bh1749nuc(bh1749nuc),_mutex(mutex)
{
    _rst = rst;
}

void LightSensor::startup(void *pvParameters)
 {
    if(pvParameters != NULL){
      pinMode(this->_rst, OUTPUT);
      digitalWrite(this->_rst,HIGH);
      ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
    }
     _mutex.lock();
    _bh1749nuc.begin();

    _mutex.unlock();
 }
 