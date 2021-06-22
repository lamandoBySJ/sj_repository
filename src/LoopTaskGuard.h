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
            _loopState =Thread::Ready;
        }
        osStatus set_signal_id(uint32_t id,bool fromISR=false){
            if(_mail_box_control.full()){
                return osError;
            }
            guard::mail_control_t* mail = _mail_box_control.alloc();
            mail->id =id;
            if(fromISR){
               return  _mail_box_control.put_from_isr(mail);
            }else{
                return _mail_box_control.put(mail);
            }
        }
        void loop(){
            
            while(true){
                if(get_loop_state()==Thread::Running){
                    LEDIndicator::getLEDIndicator().io_state_sys(true);
                    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
                    set_signal_id(1);
                    LEDIndicator::getLEDIndicator().io_state_sys(false);
                    ThisThread::sleep_for(Kernel::Clock::duration_seconds(3)); 
                }else{
                    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
                }
            }
        }
        
        osStatus get_thread_state(){
            std::lock_guard<rtos::Mutex> lck(_mtx);
           return _thread.get_state();
        }
        void set_loop_state(osStatus state){
             std::lock_guard<rtos::Mutex> lck(_mtx);
            _loopState=state;
        }

        void loop_start(){
            std::lock_guard<rtos::Mutex> lck(_mtx); 
           if(_thread.get_state()!=Thread::Running){
               _loopState=Thread::Running;
               LEDIndicator::getLEDIndicator().io_state_sys(true);
                _thread.start(mbed::callback(this,&LoopTaskGuard::loop));
           }
        }

        void loop_stop(){
            std::lock_guard<rtos::Mutex> lck(_mtx);
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
protected:
        osStatus get_loop_state(){
            std::lock_guard<rtos::Mutex> lck(_mtx);
           return _loopState;
        }
private:
        rtos::Mail<guard::mail_control_t, 6> _mail_box_control;
        rtos::Mutex _mtx;
        rtos::Thread _thread;
        osStatus _loopState;
};

} // namespace guard

#endif