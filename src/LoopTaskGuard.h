#ifndef LOOP_TASK_GUARD_H
#define LOOP_TASK_GUARD_H

#include "platform_debug.h"

namespace guard
{
    struct mail_control_t{
        uint32_t counter=0; 
        uint32_t id=0;   
    }; 
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
                ThisThread::sleep_for(Kernel::Clock::duration_seconds(6));
                TracePrinter::printTrace("-- LoopTaskGuard --");
                set_signal_id(1);
            }
        }
        void loop_start(){
            std::lock_guard<rtos::Mutex> lck(_mtx);
           if(_thread.get_state()!=Thread::Running){
               _thread.start(callback(this,&LoopTaskGuard::loop));
           }
        }
        void loop_stop(){
            std::lock_guard<rtos::Mutex> lck(_mtx);
            if(_thread.get_state()==Thread::Running){
               _thread.terminate();
           }
        }
        uint32_t get_signal_id(){
            osEvent evt;
            do{
                evt =  _mail_box_control.get();
                if(evt.status == osEventMail){
                    guard::mail_control_t* mail = (guard::mail_control_t*)evt.value.p;
                    return mail->id;
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
private:
        rtos::Mail<guard::mail_control_t, 16> _mail_box_control;
        rtos::Mutex _mtx;
        rtos::Thread _thread;
};

} // namespace guard

#endif