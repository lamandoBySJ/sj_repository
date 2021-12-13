#pragma once

#include "Arduino.h"
#include "driver/adc.h"
#include "platform_debug.h"
#include "platform/mbed.h"

namespace app{

class BatteryVoltage
{
public:
    BatteryVoltage(uint8_t adcPin=39,adc_atten_t adc_atten=ADC_ATTEN_11db):_adcPin(adcPin)
    {
      pinMode(_adcPin,INPUT);
      _full_scale_voltage=3.6;
    }
    ~BatteryVoltage(){
        
    }
    static float measure(uint8_t ctrlPin=16){
      return getInstance()._measure(ctrlPin);
    }
    static void init(uint8_t adcPin=39,adc_atten_t adc_atten=ADC_ATTEN_11db)
    { 
      /*switch(adc_atten){
        case ADC_ATTEN_DB_0   :_full_scale_voltage=1;break;
        case ADC_ATTEN_DB_2_5 :_full_scale_voltage=1.34;break;
        case ADC_ATTEN_DB_6   :_full_scale_voltage=2;break;
        case ADC_ATTEN_11db   :_full_scale_voltage=3.6;break;
        case ADC_ATTEN_MAX    :_full_scale_voltage=3.9;break;
        default:break;
      }*/

       //int read_raw;
      //adc2_config_channel_atten( ADC2_CHANNEL_4, ADC_ATTEN_11db);
      /*
      esp_err_t r = adc2_get_raw( ADC2_CHANNEL_4, ADC_WIDTH_12Bit, &read_raw);
      if ( r == ESP_OK ) {
          PlatformDebug::printf("ADC2:%f\n", read_raw*3.6/4096 );
      } else if ( r == ESP_ERR_TIMEOUT ) {
          PlatformDebug::printf("ADC2 used by Wi-Fi.\\n");
      } */ 
      getInstance()._init(adcPin,adc_atten);
    }
protected:
    void _init(uint8_t adcPin=39,adc_atten_t adc_atten=ADC_ATTEN_11db)
    { 
        std::lock_guard<rtos::Mutex> lck(_mtx);
        _adcPin=adcPin;
        _full_scale_voltage=3.6;
    }
    float _measure(uint8_t ctrlPin=16){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        pinMode(ctrlPin,OUTPUT);
        digitalWrite(ctrlPin,LOW);
        _adcValue = analogRead(_adcPin);
        PlatformDebug::printf("_adcValue:%d\n",_adcValue);
        _adcVoltage = _adcValue*_full_scale_voltage/4096-0.7;
        PlatformDebug::printf("_adcVoltage:%f\n",_adcVoltage);
        digitalWrite(ctrlPin,HIGH);
        pinMode(ctrlPin,INPUT);
        return _adcVoltage;
    }
    static BatteryVoltage& getInstance()
    { 
        static BatteryVoltage* batteryVoltage=new BatteryVoltage();
        return *batteryVoltage;
    }
private:
  rtos::Mutex _mtx;
  uint8_t _adcPin;
  uint16_t _adcValue;
  float _adcVoltage;
  float _full_scale_voltage=3.6;

};

}//end namespace app