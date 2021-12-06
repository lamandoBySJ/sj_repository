#include "../IOExpander/MCP23017Builder.h"

void MCP23017Builder::power_on()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    if(_rst==-1){
        return;
    }
    pinMode(_rst,OUTPUT);
    digitalWrite(_rst,LOW);
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    digitalWrite(_rst,HIGH);
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));
}