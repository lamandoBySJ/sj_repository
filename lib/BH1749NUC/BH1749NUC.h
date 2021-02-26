#ifndef BH1749NUC_H
#define BH1749NUC_H

#include <Arduino.h>
#include <Wire.h>

#define MANUFACTURER_ID_REG_ADDRESS           0x92U

 class BH1749NUC
 {
public:
    BH1749NUC(TwoWire& wire,uint8_t  sda,uint8_t scl,uint32_t frequency=100000):_wire(wire)
    {
            this->_sda=sda;
            this->_scl=scl;
            this->_frequency =frequency;
            this->_device_address = 0x39;
    }
    ~BH1749NUC();
    bool begin();
    
protected:
    bool platfrom_write(uint8_t reg_address, uint8_t* data,uint8_t len);
    bool platfrom_read(uint8_t reg_address, uint8_t* data,uint8_t len);
private:
    TwoWire& _wire;
    uint8_t _sda;
    uint8_t _scl;
    uint32_t _frequency;
    uint8_t _device_address;
 };


 #endif;