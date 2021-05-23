#include "ColorSensor.h"

template<typename Sensor,typename OSMutex>
ColorSensor<Sensor, OSMutex>::ColorSensor(TwoWire& wire,uint8_t  sda,uint8_t scl, OSMutex &mtx): _sensor(wire,sda,scl),_mtx(mtx),_rst(0),_measurementHook(nullptr)
{   
    
    ptrFuns[0]= &Sensor::red_data_get;
    ptrFuns[1]= &Sensor::green_data_get;
    ptrFuns[2]= &Sensor::blue_data_get;
    ptrFuns[3]= &Sensor::ir_data_get;
    _mode_control2.reg=0;
}

template<typename Sensor,typename OSMutex>
ColorSensor<Sensor, OSMutex>::ColorSensor(TwoWire& wire,uint8_t  sda,uint8_t scl,uint8_t rst,OSMutex &mtx): _sensor(wire,sda,scl),_mtx(mtx),_rst(rst),_measurementHook(nullptr)
{
    ptrFuns[0]= &Sensor::red_data_get;
    ptrFuns[1]= &Sensor::green_data_get;
    ptrFuns[2]= &Sensor::blue_data_get;
    ptrFuns[3]= &Sensor::ir_data_get;
    _mode_control2.reg=0;
}

template<typename Sensor,typename OSMutex>
void ColorSensor<Sensor, OSMutex>::init(bool pwrEnable)
 {
    std::unique_lock<OSMutex> lck(_mtx, std::defer_lock);
    lck.lock();
    if(pwrEnable){
      pinMode(this->_rst, OUTPUT);
      digitalWrite(this->_rst,HIGH);
    }
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));

    if(! _sensor.begin()){
      digitalWrite(23,HIGH);
    }
    platform_debug::TracePrinter::printTrace(String(__FILE__)+String(":")+String(__LINE__));
}

template<typename Sensor,typename OSMutex>
void ColorSensor<Sensor, OSMutex>::measurementModeActive()
{
   _sensor.measurement_active();
}
template<typename Sensor,typename OSMutex>
void ColorSensor<Sensor, OSMutex>::measurementModeInactive()
{
   _sensor.measurement_inactive();
}

template<typename Sensor,typename OSMutex>
bool ColorSensor<Sensor, OSMutex>::getRGB(RGB& rgb)
{
  int timeout=3000;
  do{
        _sensor.mode_control2_get(&_mode_control2.reg);
       if(--timeout==0){
         return false;
       }
       delay(1);
  } while (!_mode_control2.bitfield.valid);

 return ( _sensor.*ptrFuns[0])(rgb.R) &&
  ( _sensor.*ptrFuns[1])(rgb.G)&&
  ( _sensor.*ptrFuns[2])(rgb.B)&&
  ( _sensor.*ptrFuns[3])(rgb.IR);
 
}

template class ColorSensor<BH1749NUC,std::mutex>;
template class ColorSensor<BH1749NUC,rtos::Mutex>;
