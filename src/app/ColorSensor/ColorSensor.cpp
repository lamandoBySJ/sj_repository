#include "ColorSensor.h"

template<typename Sensor,typename Mutex>
ColorSensor<Sensor,Mutex>::ColorSensor(TwoWire& wire,uint8_t  sda,uint8_t scl,Mutex& mutex):_als(wire,sda,scl),_mtx(mutex),_rst(0),_measurementHook()
{   
    
    ptrFuns[0]= &Sensor::red_data_get;
    ptrFuns[1]= &Sensor::green_data_get;
    ptrFuns[2]= &Sensor::blue_data_get;
    ptrFuns[3]= &Sensor::ir_data_get;
    _mode_control2.reg=0;
}

template<typename Sensor,typename Mutex>
ColorSensor<Sensor,Mutex>::ColorSensor(TwoWire& wire,uint8_t  sda,uint8_t scl,Mutex& mutex,uint8_t rst):_als(wire,sda,scl),_mtx(mutex),_rst(rst),_measurementHook()
{
    ptrFuns[0]= &Sensor::red_data_get;
    ptrFuns[1]= &Sensor::green_data_get;
    ptrFuns[2]= &Sensor::blue_data_get;
    ptrFuns[3]= &Sensor::ir_data_get;
    _mode_control2.reg=0;
}

template<typename Sensor,typename Mutex>
void ColorSensor<Sensor,Mutex>::init(bool pwrEnable)
 {
    std::unique_lock<Mutex> lck(_mtx, std::defer_lock);
    lck.lock();
    if(pwrEnable){
      pinMode(this->_rst, OUTPUT);
      digitalWrite(this->_rst,HIGH);
    }
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));

    if(!_als.begin()){
      digitalWrite(23,HIGH);
    }
    platform_debug::TracePrinter::printTrace(String(__FILE__)+String(":")+String(__LINE__));
}

template<typename Sensor,typename Mutex>
void ColorSensor<Sensor,Mutex>::measurementModeActive()
{
  _als.measurement_active();
}
template<typename Sensor,typename Mutex>
void ColorSensor<Sensor,Mutex>::measurementModeInactive()
{
  _als.measurement_inactive();
}
Mutex mtx;
template<typename Sensor,typename Mutex>
bool ColorSensor<Sensor,Mutex>::getRGB(RGB& rgb)
{
  int timeout=3000;
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

template class ColorSensor<BH1749NUC,rtos::Mutex>;
template class ColorSensor<BH1749NUC,std::mutex>;

