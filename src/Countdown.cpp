#include "Countdown.h"

/*
void Countdown::shutdown(){
    if(_thread.get_state()==Thread::Running){
      osStatus status =   _thread.terminate();
      TracePrinter::printTrace("Timeout Checker :"+_name+" STOP!"+String((int)status,DEC));
    }
}*/


/*
void Countdown::startup(){
    if(_thread.get_state()!=Thread::Running){
      osStatus status =  _thread.start(callback(this,&Countdown::task_restart_countdown));
      TracePrinter::printTrace("Timeout Checker :"+_name+" START!"+String((int)status,DEC));
    }

}

void Countdown::task_timeout_checker(){
    while(true){
        ThisThread::flags_wait_all_for(0x0,Kernel::Clock::duration_seconds(_timeout));
        TracePrinter::printTrace("Timeout Checker :"+_name+" Restart!......");
        _decided=true;
        ThisThread::sleep_for(Kernel::Clock::duration_seconds(2));
        ESP.restart(); 
    }
}*/
