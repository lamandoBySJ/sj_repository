#include <BH1749NUC.h>


BH1749NUC::~BH1749NUC()
{

}
bool BH1749NUC::begin()
{
    bool success=false;
    _wire.begin(this->_sda, this->_scl,100000);
    _wire.beginTransmission(0x68);
    success = (_wire.endTransmission() == 0 ? true : false);
    return success;
}

bool BH1749NUC::platfrom_write(uint8_t reg_address, uint8_t* data,uint8_t len)
{
   uint8_t ret;
    _wire.beginTransmission(_device_address);
    ret= _wire.write(reg_address);
    _wire.endTransmission();   
    if(ret==true){
        _wire.requestFrom(_device_address,(int)len,1);
        ret = 0;
        if(_wire.available()){
            *data=_wire.read();
            ++data;
            ++ret;
        }
    }
    return ret==len?true:false;
 }

bool BH1749NUC::platfrom_read(uint8_t reg_address, uint8_t* data,uint8_t len)
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