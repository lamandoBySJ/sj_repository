#include "ColorSensor.h"
/*
template<typename T>
int32_t ColorSensor<T>::thunk_write_reg(void *thread_ptr)
{
  (static_cast< ColorSensor * > (thread_ptr))->_task_write();
}

template<typename T>
int32_t ColorSensor<T>::thunk_read_reg(void *thread_ptr)
{
  (static_cast< ColorSensor * > (thread_ptr))->_task_read();
}
*/

template<typename T>
ColorSensor<T>::ColorSensor(T& als,rtos::Mutex& mutex):_als(als),_mutex(mutex)
{
    _rst = 0;
}

template<typename T>
ColorSensor<T>::ColorSensor(T& als,rtos::Mutex& mutex,uint8_t rst):_als(als),_mutex(mutex)
{
    _rst = rst;
}

template<typename T>
void ColorSensor<T>::startup(void *pvParameters)
 {
    if(pvParameters != NULL){
      pinMode(this->_rst, OUTPUT);
      digitalWrite(this->_rst,HIGH);
      ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
    }
     _mutex.lock();
    _als.begin();

    _mutex.unlock();
 }
 
template class ColorSensor<BH1749NUC>;
