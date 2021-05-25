#ifndef THREAD_CONTROL_GUARD_H
#define THREAD_CONTROL_GUARD_H

#include "platform_debug.h"

namespace guard
{
    struct mail_control_t{
        uint32_t counter=0; 
        uint32_t id=0;   
    }; 
} // namespace guard

struct mail_control_t{
        uint32_t counter=0; 
        uint32_t id=0;   
}; 
class ThreadControlGuard
{
public:
        ThreadControlGuard(){
            
        }
        void startup(){

        }

        void run_controller(){

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
        
        uint32_t get_signal_id(){
            osEvent evt;
            do{
                evt =  _mail_box_control.get();
                if(evt.status == osEventMail){
                    mail_control_t* mail = (mail_control_t*)evt.value.p;
                    return mail->id;
                }else{
                    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(100));  
                }
            }while(evt.status != osEventMail);
            return -1;
        }
private:
        rtos::Mail<guard::mail_control_t, 16> _mail_box_control;
        rtos::Mutex _mtx;
};


#endif