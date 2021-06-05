#ifndef LOOP_TASK_GUARD_H
#define LOOP_TASK_GUARD_H

#include "platform_debug.h"
#include "LEDIndicator.h"
namespace guard
{
    struct mail_control_t{
        uint32_t counter=0; 
        uint32_t id=0;   
    }; 
 
class LoopTaskGuard
{
public:
        LoopTaskGuard(){
            
        }
        osStatus set_signal_id(uint32_t id,bool fromISR=false){
            //std::lock_guard<rtos::Mutex> lck(_mtx);
            guard::mail_control_t* mail = (guard::mail_control_t*)_mail_box_control.alloc();
            mail->id =id;
            if(fromISR){
               return  _mail_box_control.put_from_isr(mail);
            }else{
                return _mail_box_control.put(mail);
            }
        }
        void loop(){
            while(true){
                TracePrinter::printTrace("-- LoopTaskGuard --");
                LEDIndicator::getLEDIndicator().io_state_sys(true);
                ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
                set_signal_id(1);
                LEDIndicator::getLEDIndicator().io_state_sys(false);
                ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
            }
        }
        void loop_start(){
           if(_thread.get_state()!=Thread::Running){
               LEDIndicator::getLEDIndicator().io_state_sys(true);
                _thread.start(mbed::callback(this,&LoopTaskGuard::loop));
           }
        }
        void loop_stop(){
            if(_thread.get_state()==Thread::Running){
               _thread.terminate();
               ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(200));
               LEDIndicator::getLEDIndicator().io_state_sys(false);
           }
        }
        uint32_t get_signal_id(){
            osEvent evt;
            do{
                evt =  _mail_box_control.get();
                if(evt.status == osEventMail){
                    guard::mail_control_t* mail = (guard::mail_control_t*)evt.value.p;
                    int id=mail->id;
                    _mail_box_control.free(mail);
                    return id;
                }else{
                    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(100));  
                }
            }while(evt.status != osEventMail);
            return -1;
        }
        static LoopTaskGuard& getLoopTaskGuard(){
            static LoopTaskGuard loopTaskGuard;
            return loopTaskGuard;
        }

        static void set_signal_web_service(){
            guard::LoopTaskGuard::getLoopTaskGuard().set_signal_id(0,true);
        }
private:
        rtos::Mail<guard::mail_control_t, 3> _mail_box_control;
        rtos::Mutex _mtx;
        rtos::Thread _thread;
};

} // namespace guard

#endif