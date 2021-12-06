#include "LoRaTag.h"
#include "platform_debug.h"
#include "platformio_api.h"
#include "LoopTaskGuard.h"
#include "app/Alarm/Alarm.h"
void LoRaTag::startup()
{
_threadLoraService.get_state()==Thread::Running ? throw os::thread_error(osError):_threadLoraService.start(mbed::callback(this,&LoRaTag::run_lora_service));
_thread.get_state()==Thread::Running ? throw os::thread_error(osError):_thread.start(mbed::callback(this,&LoRaTag::task_inactive_check_service));
}

void LoRaTag::post_mail(bool fromISR,bool inactive)
{
    mail_sleep_t *mail =_mail_box_sleep.alloc();
    if(mail){
        mail->inactive = inactive;
        if(fromISR){
             _mail_box_sleep.put_from_isr(mail);
        }else{
             _mail_box_sleep.put(mail);
        }
    }
}

void LoRaTag::on_device_inactive()
{
    postMailInactiveMessage(GPIO_NUM_22);
}

void LoRaTag::task_inactive_check_service()
{
    pinMode(22, PULLUP);
    attachInterrupt(22,[]{
                detachInterrupt(22);
                (*getInstance())->post_mail(true,true);
    },FALLING);
   uint64_t mask;
	mask = 1ull << 26;

    while(true){
        osEvent evt=_mail_box_sleep.get();
        if (evt.status == osEventMail) {
            mail_sleep_t *mail = (mail_sleep_t *)evt.value.p;

            if(mail->inactive){
                TracePrinter::printTrace("--- inactive ---"+String(_mail_box_sleep.empty(),DEC));
                    if(_bufferInactive.available()==0 && _mail_box_lora.empty() &&_mail_box_sleep.empty()){
                        Alarm::close();
                        Alarm::sound(false);
                        esp_sleep_enable_ext0_wakeup(GPIO_NUM_22, 0);
                        esp_sleep_enable_ext1_wakeup(mask,ESP_EXT1_WAKEUP_ANY_HIGH);
                        gpio_wakeup_enable(GPIO_NUM_22,GPIO_INTR_LOW_LEVEL);
                        esp_sleep_enable_gpio_wakeup();
                        esp_light_sleep_start(); 
                    }
            }else{
                    TracePrinter::printTrace("--- move ---");
                    Alarm::open();
                    _io22_sleep_flipper.attach(mbed::callback(this,&LoRaTag::on_device_inactive),std::chrono::seconds(3));
                    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(600));
                    attachInterrupt(22,[]{
                        detachInterrupt(22); 
                        (*getInstance())->post_mail(true,false);
                    },FALLING);
            }
            _mail_box_sleep.free(mail); 
        }
    }
}


void LoRaTag::run_lora_service()
{

    while(true){
        
        osEvent evt= _mail_box_lora.get();
        _bufferInactive.queue(0);
        if (evt.status == osEventMail) {
            lora_tag::mail_t *mail = (lora_tag::mail_t *)evt.value.p;
            TracePrinter::printTrace("[TAG]lora:Tx:"+mail->lora_mail.sender+"Packet:"+mail->lora_mail.packet);
            if(mail->lora_mail.sender == platformio_api::get_device_info().BoardID){
                TracePrinter::printTrace("[TAG]lora:Tx:inactive");
                // _loRaService.sendMessage("FAFA",platformio_api::get_device_info().BoardID,"{\"beacon\":\"scan\"}");
            }else if(mail->lora_mail.receiver == platformio_api::get_device_info().BoardID){
                DynamicJsonDocument  doc(mail->lora_mail.packet.length()+128);
                DeserializationError error = deserializeJson(doc,mail->lora_mail.packet);
                TracePrinter::printTrace("[TAG]lora:Rx:"+mail->lora_mail.receiver);
                if (!error)
                {   
                    //packet = {"cmd":"track"}
                    TracePrinter::printTrace(doc.as<String>());
                    _loRaService.sendMessage("FAFA",platformio_api::get_device_info().BoardID,"{\"beacon\":\"scan\"}");
                }else{
                    TracePrinter::printTrace("[TAG]lora:Rx: JsonParse ERROR...");
                }
            }else if(mail->lora_mail.receiver == "FAFA"){
                TracePrinter::printTrace("[TAG]lora:Rx:FAFA");
            }else{
                TracePrinter::printTrace("[TAG]lora:Rx:[x]:n/a");
            }

            _mail_box_lora.free(mail);   
            _bufferInactive.flush(); 
            post_mail(false,true);
           
        }
    }
}

void LoRaTag::onLoRaMessageCallback(const lora::mail_t& lora_mail)
{     
    if(lora_mail.receiver==platformio_api::get_device_info().BoardID){
        lora_tag::mail_t *mail =  _mail_box_lora.alloc();
        if(mail!=NULL ){
            mail->lora_mail=lora_mail;
            mail->gpio_num = GPIO_NUM_26;
            mail->lora_mail.packet=lora_mail.packet;
            _mail_box_lora.put(mail) ;
        }
    }  
}

void LoRaTag::postMailInactiveMessage(gpio_num_t gpio_num)
{
      lora_tag::mail_t *mail =  _mail_box_lora.alloc();
      if(mail!=NULL){
        mail->lora_mail.sender = platformio_api::get_device_info().BoardID;
        mail->lora_mail.receiver= platformio_api::get_device_info().BoardID;
        mail->gpio_num = gpio_num;
        mail->lora_mail.packet="";
        _mail_box_lora.put(mail) ;
      }
}