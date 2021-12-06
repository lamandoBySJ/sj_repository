#pragma once
#include "Arduino.h"
#include <rtos/rtos.h>
enum class RadioEvent
{
    RF_IDLE =0,
    RF_CAD = 1,
    RF_SLEEP =2,
    RF_STANDBY =3,
    RF_TX_DONE =4,
    RF_RX_DONE = 5,
    RF_TX_TIMEOUT = 6,
    RF_RX_TIMEOUT = 7,
    RF_CAD_DONE = 8,
    RF_CAD_DETECTED =9
};
enum class RadioIRQ
{
    DIO0 =0,
    DIO1 =1,
    DIO2 =2,
    DIO3 =3,
    DIO4 =4,
    DIO5 =5,
    NONE =99
};
    
class RadioListener
{
public:
    RadioListener(uint32_t timeout=0):_timeout(timeout)
    {
    }
    ~RadioListener()=default;
virtual void onRadioListenerChanged()=0;
void setTimeout(uint32_t timeout = 4294967295U){
    _timeout=timeout;
}
uint32_t getTimeout(){
  return  _timeout;
}
protected:
uint32_t _timeout;
};
class OnTxRunningListener :virtual public RadioListener
{
public:
    OnTxRunningListener(uint32_t timeout=0):RadioListener(timeout){ }
    ~OnTxRunningListener()=default;
virtual void onTxRunning()=0;
void onRadioListenerChanged() override{
    onTxRunning();
}
};

class OnRxRunningListener :virtual public  RadioListener
{
public:
    OnRxRunningListener(uint32_t timeout=0):RadioListener(timeout)
    {
    }
    ~OnRxRunningListener()=default;
virtual void onRxRunning()=0;
void onRadioListenerChanged() override{
    onRxRunning();
}
};

//template<class T>
class RadioDelegator //:public TxDelegator
{
public:
    RadioDelegator()=delete;
    
    RadioDelegator(RadioListener* listener,uint32_t timeout=3000) : _listener(listener)
    {
        _listener->setTimeout(timeout);
    }
    ~RadioDelegator(){
        _listener->onRadioListenerChanged();
        _listener=nullptr;
    }
private:
    RadioListener* _listener;
};

/*
RadioListener
OnTxRunningListener
RadioListener
OnRxRunningListener
A

~A

~OnRxRunningListener
~RadioListener
~OnTxRunningListener
~RadioListener

*/