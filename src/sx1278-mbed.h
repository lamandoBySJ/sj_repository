
/**
 *  @brief:  Implementation of a SX1278 platform dependent [MBED] radio functions
 *  @author: luk6xff
 *  @email:  luszko@op.pl
 *  @date:   2019-11-15
 */


#ifndef __SX1278_MBED_H__
#define __SX1278_MBED_H__

//#include "mbed.h"
#include "Arduino.h"
#include "sx1278.h"
#include "hal/PinNames.h"
#include "registers.h"
#include "sx1278.h"
#include "drivers/DigitalInOut.h"
#include "drivers/DigitalOut.h"
#include "drivers/DigitalIn.h"
#include "drivers/InterruptIn.h"
#include "platform/mbed.h"
#include "drivers/Timeout.h"
#include "heltec.h"
#include "platform/mbed.h"
#include <atomic>
using namespace mbed;

class SX1278:public SX1278Interface,public Radio
{
public:
~SX1278()
{
    MbedDeInit();
}
SX1278()
{

}
uint8_t readVersion(){
  return SX1278Read(REG_VERSION);
}
void MbedDeInit();

void IoInit(PinName _mosi, PinName _miso, PinName _sclk, PinName _nss,
            PinName _dio0, PinName _dio1, PinName _dio2, PinName _dio3, PinName _dio4, PinName _dio5,
            PinName _reset) override;
void IoDeInit(void) override;
void WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size) override;
void ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size) override;
void DelayMs(int ms) override;
void startup()
{
   ModuleInit();
   _thread.start(mbed::callback(this,&SX1278::run_service));
}
void run_service(){
    //RadioIRQ IRQ= receive(0);
    if(_implementObjects.size()==0){
       SX1278Interface* _this=this;
       addDelegateObject(&_this);
    }
    while(true)
    {
        //sx1278.SX1278SetRx(3000);
    
     // sx1278.SX1278Send((uint8_t*)ClientMsg, sizeof(ClientMsg));
    
      SX1278SetRx(1000);

     // sx1278.DelayMs(random(1000,4000));
      //debug_if(DEBUG_ON, "> Data sent to the client:%d\n\r",sizeof(ClientMsg));
    }
}
void setSPIFrequency(uint32_t frequency)
{
  _spiSettings = SPISettings(frequency, MSBFIRST, SPI_MODE0);
}
/*
uint8_t singleTransfer(uint8_t address, uint8_t value)
{
  uint8_t response;
  *nss=0;
  spi->beginTransaction(_spiSettings);
  spi->transfer(address);
  response =spi->transfer(value);
  spi->endTransaction();
  *nss=1;
  return response;
}*/
void spi_nss(bool cs){
  cs?*nss=0:*nss=1;
}

void TxDone(void)override{
    Serial.println("mbed default callback:  TxDone(\n");
 }
 void TxTimeout(void)override{
    Serial.println("mbed default callback:  TxTimeout(\n");
 }
  void RxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)override{
      Serial.println("mbed default callback:  RxDone(\n");
  }
 void RxTimeout(void)override{
    Serial.println("mbed default callback:  RxTimeout(\n");
 }
 void RxError(void)override{
    Serial.println("mbed default callback:  RxError(\n");
 }
 void FhssChangeChannel(uint8_t currentChannel)override{
    Serial.println("mbed default callback:  FhssChangeChannel(\n");
 }
 void CadDone(bool channelActivityDetected)override{
    Serial.println("mbed default callback:  CadDone(\n");
 }
 void CadDetected ()override{
    Serial.println("mbed default callback:  CadDetected(\n");
 }
private:
    SPISettings _spiSettings;
    Thread _thread;
    SPIClass* spi; 
    //SPI* spi; // mosimiso, sclk
    DigitalOut* nss;

 
};


#endif // __SX1278_MBED_H__

