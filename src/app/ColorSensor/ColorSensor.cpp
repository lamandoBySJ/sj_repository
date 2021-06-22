#include "ColorSensor.h"

ColorSensor::ColorSensor(rtos::Mutex &mtx,TwoWire& wire,uint8_t rst):_mtx(mtx),_bh1749nuc(wire),_rst(rst)
{
    ptrFuns[0]= &BH1749NUC::red_data_get;
    ptrFuns[1]= &BH1749NUC::green_data_get;
    ptrFuns[2]= &BH1749NUC::blue_data_get;
    ptrFuns[3]= &BH1749NUC::ir_data_get;
    _mode_control2.reg=0;
}

void ColorSensor::power_on()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    pinMode(_rst,OUTPUT);
    digitalWrite(_rst,LOW);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    digitalWrite(_rst,HIGH);
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(500));
}
bool ColorSensor::init()
 {
    std::lock_guard<rtos::Mutex> lck(_mtx);
    
    if( _bh1749nuc.begin()){
      return true;
    }else{
      return false;
    }
    TracePrinter::printTrace(String(__FILE__)+String(":")+String(__LINE__));
}


void ColorSensor::measurementModeActive()
{
   _bh1749nuc.measurement_active();
}

void ColorSensor::measurementModeInactive()
{
   _bh1749nuc.measurement_inactive();
}


bool ColorSensor::getRGB(RGB& rgb)
{
  int timeout=3000;
  do{
        _bh1749nuc.mode_control2_get(&_mode_control2.reg);
       if(--timeout==0){
         return false;
       }
       delay(1);
  } while (!_mode_control2.bitfield.valid);

 return ( _bh1749nuc.*ptrFuns[0])(rgb.R) &&
  ( _bh1749nuc.*ptrFuns[1])(rgb.G)&&
  ( _bh1749nuc.*ptrFuns[2])(rgb.B)&&
  ( _bh1749nuc.*ptrFuns[3])(rgb.IR);
 
}
