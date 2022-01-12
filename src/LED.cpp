#include "LED.h"

void LED::io_state_reset()
{
    pinMode(5,OUTPUT);
    pinMode(16,OUTPUT);
    pinMode(17,OUTPUT);
    pinMode(18,OUTPUT);
    pinMode(19,OUTPUT);
    pinMode(23,OUTPUT);
    digitalWrite(5,HIGH);
    digitalWrite(16,HIGH);
    digitalWrite(17,HIGH);
    digitalWrite(18,HIGH);
    digitalWrite(19,HIGH);
    digitalWrite(23,HIGH);
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(500));
    digitalWrite(5,LOW);
    digitalWrite(16,LOW);
    digitalWrite(17,LOW);
    digitalWrite(18,LOW);
    digitalWrite(19,LOW);
    digitalWrite(23,LOW);
}
void LED::io_state(LedName name,bool ok)
{
    switch(name)
    {
        case LedName::WiFi: digitalWrite(16,ok?HIGH:LOW);break;
        case LedName::MQTT: digitalWrite(5,ok?HIGH:LOW);break;
        case LedName::RTC : digitalWrite(17,ok?HIGH:LOW);break;
        case LedName::ALS : digitalWrite(18,ok?HIGH:LOW);break;
        case LedName::WEB : digitalWrite(19,ok?HIGH:LOW);break;
        case LedName::SYS : digitalWrite(23,ok?HIGH:LOW);break;
    }
}