#ifndef BH1749NUC_H
#define BH1749NUC_H

#include <Arduino.h>
#include <Wire.h>
#include <ColorSensorBase.h>
#include <BH1749NUC_REG/bh1749nuc_reg.h>
#include <array>
//#include <platform/mbed_debug.h>
//#include <cxxsupport/mstd_type_traits.h>

#define MANUFACTURER_ID_REG_ADDRESS           0x92U

typedef enum class GainRGB : uint8_t{
    Forbidden=0,
    X1=1,
    X32=3
}GainRGB;

typedef enum class GainIR : uint8_t{
    Forbidden=0,
    X1=1,
    X32=3
}GainIR;

typedef enum class MeasMode : uint8_t{
    Forbidden=0,
    Time120ms=2,
    Time240ms=3,
    Time35ms=5
}MeasMode;

 class BH1749NUC : public ColorSensorBase
 {
public:
    BH1749NUC()=delete;
    BH1749NUC(TwoWire& wire,uint8_t  sda,uint8_t scl,uint32_t frequency=100000):_wire(wire),_facturerId(0xE0)
    {
        this->_sda=sda;
        this->_scl=scl;
        this->_frequency =frequency;
        this->_device_address = 0x39;
        _data[0] = 0;
        _data[1] = 1;
    }
    ~BH1749NUC();
    bool begin() override;
    
    
 
    void sendCommand(uint8_t reg_address,uint8_t* data,int len);
    uint8_t getManufacturerId();
    bool init(GainIR gain_ir,GainRGB gain_rgb,MeasMode meas_mode);
    void measurement_active();
    void measurement_inactive();
    bool system_control_set(uint8_t val);
    bool system_control_get(uint8_t *val);
    bool mode_control1_set(uint8_t val);
    bool mode_control1_get(uint8_t *val);
    bool ir_gain_set(uint8_t val);
    bool ir_gain_get(uint8_t *val);
    bool rgb_gain_set(uint8_t val);
    bool rgb_gain_get( uint8_t *val);
    bool measurement_mode_set(uint8_t val);
    bool measurement_mode_get( uint8_t *val);
    
    bool mode_control2_set(uint8_t val);
    bool mode_control2_get(uint8_t *val);
   
    bool red_data_get(rgb1bit16_t& rgb);
    bool green_data_get(rgb1bit16_t& rgb);
    bool blue_data_get(rgb1bit16_t& rgb);
    bool ir_data_get(rgb1bit16_t& rgb);  
    //bool data_get(uint16_t& data);

private:
    bool platform_write(uint8_t reg_address, uint8_t* data,uint8_t len);
    bool platform_read(uint8_t reg_address, uint8_t* data,uint8_t len);
    std::array<char,2> _data;
    TwoWire& _wire;
    byte _facturerId;
    uint8_t _sda;
    uint8_t _scl;
    uint32_t _frequency;
    uint8_t _device_address;
    rgb1bit16_t _reg_value;
    uint8_t _reg_addr[2];
 };

 #endif