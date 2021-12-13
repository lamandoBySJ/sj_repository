#pragma once

#include "Arduino.h"
#include "driver/adc.h"
#include "platform_debug.h"
#include "platform/mbed.h"

namespace app{

class BatteryTemprature
{
public:
    BatteryTemprature(uint8_t ctrlPin=12,uint8_t adcPin=13,adc_atten_t adc_atten=ADC_ATTEN_11db):_ctrlPin(ctrlPin),_adcPin(adcPin)
    {
      pinMode(_ctrlPin,INPUT);
      pinMode(_adcPin,INPUT);
      _full_scale_voltage=3.6;
    }
    ~BatteryTemprature(){
        
    }
    static float measure()
    {
        return getInstance()._measure();
    }
    static void init(uint8_t ctrlPin=12,uint8_t adcPin=13,adc_atten_t adc_atten=ADC_ATTEN_11db)
    { 
       getInstance()._init(ctrlPin,adcPin,adc_atten);
    }
protected:
    void _init(uint8_t ctrlPin=12,uint8_t adcPin=13,adc_atten_t adc_atten=ADC_ATTEN_11db)
    { 
       std::lock_guard<rtos::Mutex> lck(_mtx);
      _ctrlPin=ctrlPin;
      _adcPin=adcPin;
      pinMode(_ctrlPin,INPUT);
      pinMode(_adcPin,INPUT);
      _full_scale_voltage=3.6;
    }
    float _measure(){
      std::lock_guard<rtos::Mutex> lck(_mtx);
      pinMode(_ctrlPin,OUTPUT);
      digitalWrite(_ctrlPin,LOW);
      _adcValue = analogRead(_adcPin);
      PlatformDebug::printf("_adcValue:%d\n",_adcValue);
      _adcVoltage = _adcValue*_full_scale_voltage/4096;
      PlatformDebug::printf("_adcVoltage:%f\n",_adcVoltage);
      _resistor = 10.0/(3.3/_adcVoltage-1);
      PlatformDebug::printf("_resistor:%f\n",_resistor);
      _temperature = -26.56*log(_resistor)+86.156661;
      PlatformDebug::printf("_temperature:%f\n",_temperature);
      digitalWrite(_ctrlPin,HIGH);
      pinMode(_ctrlPin,INPUT);
      return _temperature;
    }
    static BatteryTemprature& getInstance()
    { 
        static BatteryTemprature* battTemp=new BatteryTemprature();
        return *battTemp;
    }
private:
  rtos::Mutex _mtx;
  uint8_t  _ctrlPin;
  uint8_t _adcPin;
  float _temperature;
  uint16_t _adcValue;
  float _resistor;
  float _adcVoltage;
  float _full_scale_voltage=3.6;
};

}//end namespace app