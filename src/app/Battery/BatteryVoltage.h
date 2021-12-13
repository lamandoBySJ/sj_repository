#pragma once

#include "Arduino.h"
#include "driver/adc.h"
#include "platform_debug.h"
#include "platform/mbed.h"

namespace app{
enum class PowerType: char
{
  USB,
  BAT
};
class BatteryVoltage
{
public:
    BatteryVoltage(uint8_t adcPin=39,adc_atten_t adc_atten=ADC_ATTEN_11db):_adcPin(adcPin)
    {
      pinMode(_adcPin,INPUT);
      _full_scale_voltage=3.6;
      _powerUsb=0;
      _powerBat=0;
    }
    ~BatteryVoltage(){
        
    }
    static float measure(uint8_t ctrlPin=16){
      return getInstance()._measure(ctrlPin);
    }
    static void init(uint8_t csUsbPin=16,uint8_t csBatPin=17,uint8_t adcPin=39,adc_atten_t adc_atten=ADC_ATTEN_11db)
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
    static float USBPower(){
      return getInstance().getPower(PowerType::USB);
    }
    static float BATPower(){
      return getInstance().getPower(PowerType::BAT);
    }
protected:
    float getPower(PowerType type){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        if(type==PowerType::USB){
            return _powerUsb;
        }else{
          return _powerBat;
        }
    }
    void _init(uint8_t csUsbPin=16,uint8_t csBatPin=17,uint8_t adcPin=39,adc_atten_t adc_atten=ADC_ATTEN_11db)
    { 
        std::lock_guard<rtos::Mutex> lck(_mtx);
        pinMode(csUsbPin,INPUT);
        pinMode(csBatPin,INPUT);
        _adcPin=adcPin;
        _full_scale_voltage=3.6;
    }
    float _measure(uint8_t csPin=16){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        pinMode(csPin,OUTPUT);
        digitalWrite(csPin,LOW);
        _adcValue = analogRead(_adcPin);
        PlatformDebug::printf("_adcValue:%d\n",_adcValue);
        _adcVoltage = 2*_adcValue*_full_scale_voltage/4096+0.251;
        PlatformDebug::printf("_adcVoltage:%f\n",_adcVoltage);
        if(csPin==16){
          _powerUsb= _adcVoltage<4.7?0: (_adcVoltage-4.7)*1000/4;
        }else{
          _powerBat= _adcVoltage<3.3?0: (_adcVoltage-3.3)*100/0.9;
        }
        digitalWrite(csPin,HIGH);
        pinMode(csPin,INPUT);
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
  float _powerUsb;
  float _powerBat;
  float _adcVoltage;
  float _full_scale_voltage=3.6;

};

}//end namespace app