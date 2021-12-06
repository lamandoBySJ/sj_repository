#include "IPSIndicator.h"

IPSIndicator::IPSIndicator()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    pinMode(5,OUTPUT);
    pinMode(18,OUTPUT);
    pinMode(19,OUTPUT);
    pinMode(22,PULLUP);
    pinMode(23,OUTPUT);
}
void IPSIndicator::io_state_reset()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    digitalWrite(5,HIGH);
    digitalWrite(18,HIGH);
    digitalWrite(19,HIGH);
    digitalWrite(22,HIGH);
    digitalWrite(23,HIGH);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    digitalWrite(5,LOW);
    digitalWrite(18,LOW);
    digitalWrite(19,LOW);
    digitalWrite(22,LOW);
    digitalWrite(23,LOW);
}
   
void IPSIndicator::io_state_wifi(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    //ok?digitalWrite(16,HIGH):digitalWrite(16,LOW);
}
void IPSIndicator::io_state_mqtt(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    //ok?digitalWrite(5,HIGH):digitalWrite(5,LOW);
}
void IPSIndicator::io_state_rtc(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
   // ok?digitalWrite(17,HIGH):digitalWrite(17,LOW);
}
void IPSIndicator::io_state_als(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    //ok?digitalWrite(18,HIGH):digitalWrite(18,LOW);
}
void IPSIndicator::io_state_web(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    //ok?digitalWrite(19,HIGH):digitalWrite(19,LOW);
}
void IPSIndicator::io_state_sys(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    //ok?digitalWrite(23,HIGH):digitalWrite(23,LOW);
}