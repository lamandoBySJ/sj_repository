#pragma once
#include "platform_debug.h"
#include "LoRaService.h"
#include "drivers/Timeout.h"
#include <atomic>
//#include "rtos/Queue.h"
#include "app/CBuffer/CBuffer.h"

namespace lora_tag
{
struct mail_t
{
  mail_t(){
    gpio_num = GPIO_NUM_0;
  }
  ~mail_t(){
 
  }
  lora::mail_t& operator=(const lora::mail_t& mail)
  {
      lora_mail.rssi    =mail.rssi;
      lora_mail.sender  =mail.sender;
      lora_mail.receiver=mail.receiver;
      lora_mail.packet  =mail.packet;
      return lora_mail;
  }
  gpio_num_t gpio_num;
  lora::mail_t lora_mail;
};

}

typedef struct 
{
gpio_num_t gpio_num;
bool inactive=false;
}mail_sleep_t;

class LoRaTag
{
public:
    LoRaTag(LoRaService &loRaService):_loRaService(loRaService)
    ,_threadLoraService(osPriorityNormal,1024*6)
    {
        (*getInstance())=this;
    }
    void startup();
    void run_lora_service();
    void onLoRaMessageCallback(const lora::mail_t& lora_mail);
    void postMailInactiveMessage(gpio_num_t gpio_num);

    void on_device_inactive();
    void task_inactive_check_service();
    void post_mail(bool fromISR=false,bool inactive=false);
 
    static LoRaTag** getInstance()
    {
        static LoRaTag* _this;
        return &_this;
    }
    void shutdown(){
        if(_threadLoraService.get_state()==Thread::Running){
            _threadLoraService.terminate();
            _thread.terminate();
            _io22_sleep_flipper.detach();
        }
    }
private:
    LoRaService& _loRaService;
    rtos::Mail<lora_tag::mail_t,16> _mail_box_lora;
    rtos::Mail<mail_sleep_t,8> _mail_box_sleep;
    rtos::Thread _threadLoraService;
    rtos::Thread _thread;
    mbed::Timeout _io22_sleep_flipper;
    CBuffer<char> _bufferInactive;
};