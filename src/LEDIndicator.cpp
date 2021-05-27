#include "LEDIndicator.h"
LEDIndicator::LEDIndicator()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    pinMode(5,OUTPUT);
    pinMode(16,OUTPUT);
    pinMode(17,OUTPUT);
    pinMode(18,OUTPUT);
    pinMode(19,OUTPUT);
    pinMode(23,OUTPUT);
}
void LEDIndicator::io_state_reset()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    digitalWrite(5,HIGH);
    digitalWrite(16,HIGH);
    digitalWrite(17,HIGH);
    digitalWrite(18,HIGH);
    digitalWrite(19,HIGH);
    digitalWrite(23,HIGH);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    digitalWrite(5,LOW);
    digitalWrite(16,LOW);
    digitalWrite(17,LOW);
    digitalWrite(18,LOW);
    digitalWrite(19,LOW);
    digitalWrite(23,LOW);
}
   
void LEDIndicator::io_state_wifi(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    ok?digitalWrite(16,HIGH):digitalWrite(16,LOW);
}
void LEDIndicator::io_state_mqtt(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    ok?digitalWrite(5,HIGH):digitalWrite(5,LOW);
}
void LEDIndicator::io_state_rtc(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    ok?digitalWrite(17,HIGH):digitalWrite(17,LOW);
}
void LEDIndicator::io_state_als(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    ok?digitalWrite(18,HIGH):digitalWrite(18,LOW);
}
void LEDIndicator::io_state_web(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    ok?digitalWrite(19,HIGH):digitalWrite(19,LOW);
}
void LEDIndicator::io_state_sys(bool ok)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    ok?digitalWrite(23,HIGH):digitalWrite(23,LOW);
}