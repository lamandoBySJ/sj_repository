#include "LoRaService.h"

static char* recipient=new char[5]{0};
static char* sender=new char[5]{0};
static char* packet=new char[255]{0};
std::atomic<long> LoRaService::_lastSendTime(0);
void LoRaService::_thunkOnReceice(int packetSize)
{
    //std::stringstream recipient;
    for(char i=0;i<4;i++){
       // recipient<< (byte) LoRa.read();   
       recipient[i]=(byte)LoRa.read();    
    }

    //std::stringstream  sender;
    for(char i=0;i<4;i++){
       // sender<<  (byte)  LoRa.read();     
         sender[i]=(byte)LoRa.read(); 
    }
                           
    byte incomingLength= LoRa.read();  
    byte cnt=0; 
    for (cnt;LoRa.available()&& cnt<254;){
        packet[cnt++]= (char) LoRa.read();
    }
    
    if(incomingLength == cnt){
        lora::mail_t *mail =  (*getLoRaService())->_mail_box.alloc();
        if(mail!=NULL){
            mail->rssi = LoRa.packetRssi();
            mail->sender = String(sender);
            mail->receiver = String(recipient);
            mail->packet = packet;
            (*getLoRaService()) ->_mail_box.put_from_isr(mail) ;
        }
    }
    LoRaService::_lastSendTime = millis();
}

void LoRaService::startup()
{
    (*getLoRaService()) =this;
    LoRaService::_lastSendTime = millis();
    
    LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
    LoRa.onReceive(&LoRaService::_thunkOnReceice);
    LoRa.receive();
    _thread.start(mbed::callback(this,&LoRaService::run));
    
}
void LoRaService::run()
{
    DynamicJsonDocument  doc(1024);
    while(true){
        osEvent evt= _mail_box.get();
        if (evt.status == osEventMail) {
            lora::mail_t *mail = (lora::mail_t *)evt.value.p;
            for(auto& v :_onMessageCallbacks){ 
                v.call(*mail);
            }
            _mail_box.free(mail); 
        }
    }
}

void LoRaService::addOnMessageCallback(mbed::Callback<void(const lora::mail_t&)> func)
{
    _onMessageCallbacks.push_back(func);
}

void LoRaService::sendMessage(const String& receiver,const String& sender,const String& packet)
{   
    std::lock_guard<rtos::Mutex> lck(_mtx);
    char retry=60;
    while( (millis()-LoRaService::_lastSendTime) < 600 && --retry>0){
        TracePrinter::printTrace("       < delay >      ("+String(retry,DEC)+")");
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(random(100,200)));
    }
    LoRa.beginPacket();  
    //LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
    for(char i=0;i<4;++i){
        LoRa.write((char)receiver[i]);   
    }                      
    for(char i=0;i<4;++i){
        LoRa.write((char)sender[i]);          
    }
    LoRa.write((uint8_t)packet.length());
    LoRa.print(packet);           
    LoRa.endPacket();            
    LoRa.receive();                  
}
