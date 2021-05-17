#include "ColorSensor.h"

template<typename T>
ColorSensor<T>::ColorSensor(TwoWire& wire,uint8_t  sda,uint8_t scl,std::mutex& mutex):_als(wire,sda,scl),_mtx(mutex),_rst(0)
{   
    
    ptrFuns[0]= &T::red_data_get;
    ptrFuns[1]= &T::green_data_get;
    ptrFuns[2]= &T::blue_data_get;
    ptrFuns[3]= &T::ir_data_get;
    _mode_control2.reg=0;
}

template<typename T>
ColorSensor<T>::ColorSensor(TwoWire& wire,uint8_t  sda,uint8_t scl,std::mutex& mutex,uint8_t rst):_als(wire,sda,scl),_mtx(mutex),_rst(rst)
{
    ptrFuns[0]= &T::red_data_get;
    ptrFuns[1]= &T::green_data_get;
    ptrFuns[2]= &T::blue_data_get;
    ptrFuns[3]= &T::ir_data_get;
    _mode_control2.reg=0;
}

template<typename T>
void ColorSensor<T>::init(bool pwrEnable)
 {
   //  std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
  //  lck.lock();
    if(pwrEnable){
      pinMode(this->_rst, OUTPUT);
      digitalWrite(this->_rst,HIGH);
    }
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));

    if(!_als.begin()){
      digitalWrite(23,HIGH);
    }

    
}

template<typename T>
void ColorSensor<T>::measurementModeActive()
{
  _als.measurement_active();
}
template<typename T>
void ColorSensor<T>::measurementModeInactive()
{
  _als.measurement_inactive();
}
std::mutex mtx;
template<typename T>
bool ColorSensor<T>::getRGB(RGB& rgb)
{
  int timeout=3000;
 std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
lck.lock();

  do{
       _als.mode_control2_get(&_mode_control2.reg);
       if(--timeout==0){
         return false;
       }
       delay(1);
  } while (!_mode_control2.bitfield.valid);

 return (_als.*ptrFuns[0])(rgb.R) &&
  (_als.*ptrFuns[1])(rgb.G)&&
  (_als.*ptrFuns[2])(rgb.B)&&
  (_als.*ptrFuns[3])(rgb.IR);
 
}

template class ColorSensor<BH1749NUC>;

