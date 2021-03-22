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
ColorSensor<T>::ColorSensor(T& als,rtos::Mutex& mutex):_colorSensor(als),_mutex(mutex)
{
    _rst = 0;
}

template<typename T>
ColorSensor<T>::ColorSensor(T& als,rtos::Mutex& mutex,uint8_t rst):_colorSensor(als),_mutex(mutex)
{
    _rst = rst;
}

template<typename T>
void ColorSensor<T>::startup(bool pwrEnable)
 {
    if(pwrEnable){
      pinMode(this->_rst, OUTPUT);
      digitalWrite(this->_rst,HIGH);
    }
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    ptrFuns[0]= &T::red_data_get;
    ptrFuns[1]= &T::green_data_get;
    ptrFuns[2]= &T::blue_data_get;
    ptrFuns[3]= &T::ir_data_get;
     _mutex.lock();
   bool cuccess =  _colorSensor.begin();
    _mutex.unlock();

    if(!cuccess){
      for(auto& v : _delegateCallbacks){
          v.call(ExceptionType::ALSException,String(__FILE__)+String(":")+String(__LINE__));
      }
    }else{
      digitalWrite(5,HIGH);
    }
}

template<typename T>
void ColorSensor<T>::attach(Callback<void(ExceptionType,String)> func)
{
    _delegateCallbacks.push_back(func);
}

template<typename T>
void ColorSensor<T>::measurementModeActive()
{
  _colorSensor.measurement_active();
}
template<typename T>
void ColorSensor<T>::measurementModeInactive()
{
  _colorSensor.measurement_inactive();
}

template<typename T>
bool ColorSensor<T>::getRGB(std::array<uint16_t,4>& data)
{
  int timeout=3000;
  
  do{
       _mutex.lock();
       _colorSensor.mode_control2_get(&_mode_control2.reg);
        _mutex.unlock();

       if(--timeout==0){
         return false;
       }
       delay(1);
       
    } while (!_mode_control2.bitfield.valid);
 

  auto ptr = data.begin();
  return std::all_of(ptrFuns.begin(),ptrFuns.end(),[&] (callbackFun& p){
    _mutex.lock();
    bool result = (_colorSensor.*p)(*ptr);
    ptr++;
    _mutex.unlock();
    return result;
  });
}
/*
template<>
class ColorSensor<ColorSensorBase>
{
public:
    ColorSensor()=delete;
    ColorSensor(ColorSensorBase* csb,rtos::Mutex& mutex):_csb(csb),_colorSensor(*_csb),_mutex(mutex)
    {
        
    }
    ColorSensor(ColorSensorBase* csb,rtos::Mutex& mutex,uint8_t rst):_csb(csb),_colorSensor(*_csb),_mutex(mutex)
    {

    }
    ~ColorSensor()=default;
    void startup(bool pwrEnable=true)
    {
          debug("BH1749NUCBase..........................\n");
    }
private:

    ColorSensorBase* _csb;
    ColorSensorBase& _colorSensor;
    rtos::Mutex& _mutex;
    uint8_t _rst;
};
*/
template class ColorSensor<BH1749NUC>;

