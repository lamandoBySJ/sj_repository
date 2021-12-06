/**
 *  @brief:  Implementation of a SX1278 platform dependent [MBED] radio functions
 *  @author: luk6xff
 *  @email:  luszko@op.pl
 *  @date:   2019-11-15
 */

#include "sx1278-mbed.h"
#include "drivers/DigitalInOut.h"
#include "drivers/DigitalOut.h"
#include "drivers/DigitalIn.h"
#include "drivers/InterruptIn.h"
//#include "sx1278-mbed.h"
#include "drivers/Timeout.h"
#include "heltec.h"
#include "platform/mbed.h"
/**
 * Tx and Rx timers
 */
Timeout txTimeoutTimer;
Timeout rxTimeoutTimer;
Timeout rxTimeoutSyncWord;
//-----------------------------------------------------------------------------
void SX1278::MbedDeInit()
{
    // IO
    IoDeInit();
    // Timers
    txTimeoutTimer.detach();
    rxTimeoutTimer.detach();
    rxTimeoutSyncWord.detach();
}
//-----------------------------------------------------------------------------
void SX1278::IoInit(PinName _mosi, PinName _miso, PinName _sclk, PinName _nss,
            PinName _dio0, PinName _dio1, PinName _dio2, PinName _dio3, PinName _dio4, PinName _dio5,
            PinName _reset)
{
    

    reset = new DigitalOut(_reset);
    dio0 = new InterruptIn(_dio0);
    dio1 = new InterruptIn(_dio1);
    dio2 = new InterruptIn(_dio2);
   //dio3 = new InterruptIn(_dio3);
   //dio4 = new InterruptIn(_dio4);
   //dio5 = new DigitalIn(_dio5);

    nss = new DigitalOut(_nss);
    // spi = new SPI(_mosi, _miso, _sclk);
    Reset();
    // Init SPI
    spi = &SPI;
    *nss = 1; 
    //spi->format(8,0);   
    uint32_t frequencyToSet = 8000000;
    setSPIFrequency(frequencyToSet);
    //spi->frequency(frequencyToSet);
    //spi->setFrequency(frequencyToSet);
    spi->begin(_sclk,_miso,_mosi,_nss); 
    DelayMs(100);
}

//-----------------------------------------------------------------------------
void SX1278::IoDeInit(void)
{
    //delete(spi);
    spi=nullptr;
    delete(nss);
    delete(reset); 
    delete(dio0);
    delete(dio1);
    delete(dio2);
    delete(dio3);
    delete(dio4);
    delete(dio5);
}

//-----------------------------------------------------------------------------
void SX1278::WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    uint8_t i;

    *nss = 0;
    //uint8_t  response;
   // spi->write(addr | 0x80);
   spi->beginTransaction(_spiSettings);
   spi->transfer(addr | 0x80);
    for(i = 0; i < size; i++)
    {
        //spi->write(buffer[i]);
       // response = spi->transfer(buffer[i]);
        spi->transfer(buffer[i]);
    }
     spi->endTransaction();


    *nss = 1;
}

//-----------------------------------------------------------------------------
void SX1278::ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
    uint8_t i;

    *nss = 0;
    //spi->write(addr & 0x7F);
     spi->beginTransaction(_spiSettings);
    spi->transfer(addr & 0x7F);
    for(i = 0; i < size; i++)
    {
       // buffer[i] = spi->write(0);
      // spi->write(0);
       buffer[i] = spi->transfer(0x00);
    }
    spi->endTransaction();
    *nss = 1;
}

//-----------------------------------------------------------------------------
void SX1278::DelayMs(int ms)
{
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(ms));
}