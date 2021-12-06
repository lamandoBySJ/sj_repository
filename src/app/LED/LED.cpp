#include "LED.h"
#include "app/IOExpander/MCP.h"

void LED::io_state_reset()
{
    pinMode(2,OUTPUT);
    
    digitalWrite(2,HIGH);
   // digitalWrite(21,HIGH);
    //digitalWrite(23,HIGH);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
   // digitalWrite(21,LOW);
   // digitalWrite(23,LOW);
   digitalWrite(2,LOW);
}
   
void LED::io_state(LedName name,bool ok)
{
    switch(name)
    {
        case LedName::WiFi:
        {
            //digitalWrite(21,ok?HIGH:LOW);
        }
        break;
        case LedName::MQTT:break;
        case LedName::RTC:break;
        case LedName::ALS:break;
        case LedName::WEB:
        {
            digitalWrite(2,ok?HIGH:LOW);
        }
        break;
        case LedName::SYS:
        {
             digitalWrite(2,ok?HIGH:LOW);
        }
        break;
    }
    
}
