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
ColorSensor<T>::ColorSensor(T& als,std::mutex& mutex):_colorSensor(als),_mtx(mutex)
{
    _rst = 0;
}

template<typename T>
ColorSensor<T>::ColorSensor(T& als,std::mutex& mutex,uint8_t rst):_colorSensor(als),_mtx(mutex)
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
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    bool cuccess =  _colorSensor.begin();
    
    if(cuccess){
      digitalWrite(5,HIGH);
    }else{
      digitalWrite(5,LOW);
    }
    platform_debug::TracePrinter::printTrace(String(cuccess ? "OK":"ERROR:"+String(__FILE__)+String(":")+String(__LINE__)));
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
bool ColorSensor<T>::getRGB(RGB& rgb)
{
  int timeout=3000;
 std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
  _mutex.lock();

  do{
       _colorSensor.mode_control2_get(&_mode_control2.reg);
       if(--timeout==0){
         return false;
       }
       delay(1);
  } while (!_mode_control2.bitfield.valid);

 return (_colorSensor.*ptrFuns[0])(rgb.R) &&
  (_colorSensor.*ptrFuns[1])(rgb.G)&&
  (_colorSensor.*ptrFuns[2])(rgb.B)&&
  (_colorSensor.*ptrFuns[3])(rgb.IR);

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

