#include "TimeoutChecker.h"

void TimeoutChecker::startup(){
    _thread.start(callback(this,&TimeoutChecker::task_restart_countdown));
}
void TimeoutChecker::task_restart_countdown(){
    while(true){
        ThisThread::flags_wait_all_for(0x0,Kernel::Clock::duration_seconds(_timeout));
        TracePrinter::printTrace("Timeout Checker : Restart!......");
        _decided=true;
        ThisThread::sleep_for(Kernel::Clock::duration_seconds(2));
        ESP.restart(); 
    }
}
bool TimeoutChecker::makesDecision(uint32_t flags){
    _thread.flags_set(flags);
    return _decided;
}