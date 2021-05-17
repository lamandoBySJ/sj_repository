#include <BH1749NUC.h>

//extern inline void debug(const char *format, ...);
//extern inline void debug_if(int condition, const char *format, ...) ;

BH1749NUC::~BH1749NUC()
{

}
uint8_t BH1749NUC::getManufacturerId()
{
     // initI2cIfNeccesary();
      _wire.beginTransmission(_device_address);
      _wire.write(MANUFACTURER_ID_REG_ADDRESS);
      _wire.endTransmission();
      uint8_t  data = _wire.requestFrom((int)_device_address,1,1);
      if(_wire.available()){
        data = _wire.read();
       // _wire.flush();
      }
      return data; 
}
bool BH1749NUC::begin()
{
    bool success=false;
    _wire.begin(this->_sda, this->_scl,100000);
    _wire.beginTransmission(0x68);
    _wire.endTransmission() ;

    byte whoamI= getManufacturerId();

    if(whoamI == _facturerId){
        success=true;
        init(GainIR::X1, GainRGB::X1, MeasMode::Forbidden);
    }

    return success;
}
bool BH1749NUC::init(GainIR gain_ir,GainRGB gain_rgb,MeasMode meas_mode)
{
    bh1749nuc_system_control_t system_control;
    system_control.bitfield.sw_reset=1;
    system_control.bitfield.int_reset=1;
    system_control_set(system_control.reg);
    delay(200);
    bh1749nuc_mode_control1_t mode_control1;
    mode_control1.bitfield.ir_gain=static_cast<uint8_t>(gain_ir);
    mode_control1.bitfield.rgb_gain=static_cast<uint8_t>(gain_rgb);
    mode_control1.bitfield.measurement_mode=static_cast<uint8_t>(meas_mode);

 
    mode_control1_set(mode_control1.reg);
    mode_control1_get(&mode_control1.reg);
    return true;
}

void BH1749NUC::measurement_active()
{
    //enable sensor
    bh1749nuc_mode_control2_t mode_control2;
    mode_control2.bitfield.rgb_en=1;
    mode_control2_set(mode_control2.reg);
}

void BH1749NUC::measurement_inactive()
{
    //enable sensor
    bh1749nuc_mode_control2_t mode_control2;
    mode_control2.bitfield.rgb_en=0;
    mode_control2_set(mode_control2.reg);
}
bool BH1749NUC::system_control_set(uint8_t val)
{
    return platform_write(BH1749NUC_SYSTEM_CONTROL_REG_ADDR, &val, 1);
}

bool BH1749NUC::system_control_get(uint8_t *val)
{
    platform_read(BH1749NUC_SYSTEM_CONTROL_REG_ADDR,val, 1);
    return true; 
}
bool BH1749NUC::mode_control1_set(uint8_t val)
{
    return platform_write(BH1749NUC_MODE_CONTROL1_REG_ADDR, &val, 1);
}

bool BH1749NUC::mode_control1_get(uint8_t *val)
{
    platform_read(BH1749NUC_MODE_CONTROL1_REG_ADDR,val, 1);
    return true; 
}

bool BH1749NUC::ir_gain_set(uint8_t val)
{
        uint8_t dummy;
        bool ret = platform_read(BH1749NUC_MODE_CONTROL1_REG_ADDR, &dummy, 1);
        if (true == ret ) {
          return platform_write(BH1749NUC_MODE_CONTROL1_REG_ADDR, &(val), 1);
        }
    
        return ret;
}
bool BH1749NUC::ir_gain_get(uint8_t *val)
{
        bool ret = platform_read( BH1749NUC_MODE_CONTROL1_REG_ADDR, val, 1);
        return ret;
}

bool BH1749NUC::rgb_gain_set(uint8_t val)
{
        uint8_t dummy;
        bool ret = platform_read(BH1749NUC_MODE_CONTROL1_REG_ADDR, &dummy, 1);
        if (true == ret) {
            return platform_write(BH1749NUC_MODE_CONTROL1_REG_ADDR, &(val), 1);
        }
        return ret;
}
bool BH1749NUC::rgb_gain_get( uint8_t *val)
{
    return platform_read( BH1749NUC_MODE_CONTROL1_REG_ADDR, val, 1);       
}

bool  BH1749NUC::measurement_mode_set(uint8_t val)
{
       uint8_t dummy;
        bool ret;
        ret = platform_read(BH1749NUC_MODE_CONTROL1_REG_ADDR,&dummy, 1);
    
        if (ret == true) {
            ret = platform_write(BH1749NUC_MODE_CONTROL1_REG_ADDR,&val, 1);
        }
        return ret;
}
bool BH1749NUC::measurement_mode_get(uint8_t *val)
{
   return platform_read(BH1749NUC_MODE_CONTROL1_REG_ADDR, val, 1);
}


bool BH1749NUC::mode_control2_set(uint8_t val)
{
      return  platform_write(BH1749NUC_MODE_CONTROL2_REG_ADDR, &val, 1);
}
bool BH1749NUC::mode_control2_get(uint8_t *val)
{
        return platform_read(BH1749NUC_MODE_CONTROL2_REG_ADDR, val, 1);
}

/*
bool BH1749NUC::data_get(uint16_t& data)
{
       // auto p = std::begin(_reg_value.u8bit);
        
        bool success = std::all_of(std::begin(_reg_addr),std::end(_reg_addr),[&] (uint8_t& val){
            uint8_t tmp;
            bool ret = platform_read(val, &tmp, 1);
            Serial.println("HEX"+String(val,HEX));
            Serial.println("DEC"+String(tmp,DEC));
            _reg_value = tmp;
            return ret;
        });
        data = _reg_value.i16bit;
        return success;
}
*/
bool BH1749NUC::red_data_get(reg_uint16_t& rgb)
{   
        /*
        bool ret;
        ret = platform_read(BH1749NUC_RED_DATA_L8BIT_REG_ADDR, (uint8_t*)&_reg_value.u8bit[0], 1);
        ret = platform_read(BH1749NUC_RED_DATA_H8BIT_REG_ADDR, (uint8_t*)&_reg_value.u8bit[1], 1);
        data=_reg_value.i16bit;
        return ret;
        */
    return std::all_of(_data.begin(),_data.end(),[&] (char& index){
        return platform_read(index==0?BH1749NUC_RED_DATA_L8BIT_REG_ADDR:BH1749NUC_RED_DATA_H8BIT_REG_ADDR, (uint8_t*)&rgb.u8bit[index], 1);
    });
}

bool BH1749NUC::green_data_get(reg_uint16_t& rgb)
{    
    /*
    bool ret;
    ret = platform_read(BH1749NUC_GREEN_DATA_L8BIT_REG_ADDR, (uint8_t*)&_reg_value.u8bit[0], 1);
    ret = platform_read(BH1749NUC_GREEN_DATA_H8BIT_REG_ADDR, (uint8_t*)&_reg_value.u8bit[1], 1);
    data=_reg_value.i16bit;
    return ret;
    */   
   return std::all_of(_data.begin(),_data.end(),[&] (char& index){
        return platform_read(index==0?BH1749NUC_GREEN_DATA_L8BIT_REG_ADDR:BH1749NUC_GREEN_DATA_H8BIT_REG_ADDR, (uint8_t*)&rgb.u8bit[index], 1);
    });
}

bool BH1749NUC::blue_data_get(reg_uint16_t& rgb)
{
    /*
    bool ret;
    ret = platform_read(BH1749NUC_BLUE_DATA_L8BIT_REG_ADDR, (uint8_t*)&_reg_value.u8bit[0], 1);
    ret = platform_read(BH1749NUC_BLUE_DATA_H8BIT_REG_ADDR, (uint8_t*)&_reg_value.u8bit[1], 1);
    data=_reg_value.i16bit;
    return ret;
    */   
    return std::all_of(_data.begin(),_data.end(),[&] (char& index){
        return platform_read(index==0?BH1749NUC_BLUE_DATA_L8BIT_REG_ADDR:BH1749NUC_BLUE_DATA_H8BIT_REG_ADDR, (uint8_t*)&rgb.u8bit[index], 1);
    });
}
    
bool BH1749NUC::ir_data_get(reg_uint16_t& rgb)
{
    /*
    bool ret;
    ret = platform_read(BH1749NUC_IR_DATA_L8BIT_REG_ADDR, (uint8_t*)&_reg_value.u8bit[0], 1);
    ret = platform_read(BH1749NUC_IR_DATA_H8BIT_REG_ADDR, (uint8_t*)&_reg_value.u8bit[1], 1);
    data=_reg_value.i16bit;
    return ret;
    */   
    return std::all_of(_data.begin(),_data.end(),[&] (char& index){
        return platform_read(index==0?BH1749NUC_IR_DATA_L8BIT_REG_ADDR:BH1749NUC_IR_DATA_H8BIT_REG_ADDR, (uint8_t*)&rgb.u8bit[index], 1);
    });
}

bool BH1749NUC::platform_read(uint8_t reg_address, uint8_t* data,uint8_t len)
{
   uint8_t ret;
    _wire.beginTransmission(_device_address);
    _wire.write(reg_address);
    _wire.endTransmission();   

    _wire.requestFrom((int)_device_address,(int)len,1);
    ret = 0;
    if(_wire.available()){
            *data=_wire.read();
            ++data;
            ++ret;
    }
   
    return ret==len;
 }

bool BH1749NUC::platform_write(uint8_t reg_address, uint8_t* data,uint8_t len)
{
    uint8_t ret;
     _wire.beginTransmission(_device_address);
    ret=_wire.write(reg_address);
    if(ret==1){
        for(int i=0;i<len;i++){
             ret=_wire.write(data[i]);
        }
    }
    _wire.endTransmission();
    return ret==len?true:false;
}

void BH1749NUC::sendCommand(uint8_t reg_address,uint8_t* data,int len)
{
     // initI2cIfNeccesary();
      _wire.beginTransmission(_device_address);
      _wire.write(reg_address);
      for(int i=0;i<len;i++){
         _wire.write(data[i]);
      }
      _wire.endTransmission();
}