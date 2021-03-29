#include "LoRaNetwork.h"


using namespace platform_debug;

LoRaNetwork* LoRaNetwork::_loraNetwork;
long LoRaNetwork::_lastSendTime = 0;
void LoRaNetwork::_thunkOnReceice(int packetSize)
{
    std::stringstream recipient;
    for(char i=0;i<4;i++){
        recipient<< (byte) LoRa.read();          
    }
 
    std::stringstream  sender;
    for(char i=0;i<4;i++){
        sender<<  (byte)  LoRa.read();          
    }
                           
    byte incomingLength= LoRa.read();   
   
    String  packet ="";
    while (LoRa.available()){
            packet += (char) LoRa.read();
    }
    
    if(incomingLength == packet.length()){
        lora::mail_t *mail =   _loraNetwork->_mail_box.alloc();
        if(mail!=NULL){
                mail->rssi = LoRa.packetRssi();
                mail->sender = String(sender.str().c_str());
                mail->receiver = String(recipient.str().c_str());
                mail->packet = packet;
                _loraNetwork->_mail_box.put_from_isr(mail) ;
        }
    }
    LoRaNetwork::_lastSendTime = millis();
}

void LoRaNetwork::startup()
{
    LoRaNetwork::_lastSendTime = millis();
    _loraNetwork=this;
    LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
    LoRa.onReceive(&LoRaNetwork::_thunkOnReceice);
    LoRa.receive();
    _thread.start(callback(this,&LoRaNetwork::run));
    
}
void LoRaNetwork::run()
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

void LoRaNetwork::addOnMessageCallback(Callback<void(const lora::mail_t&)> func)
{
    _onMessageCallbacks.push_back(func);
}

void LoRaNetwork::sendMessage(const String& receiver,const String& sender,const String& packet)
{   
    char retry=6;
    while( (millis()-LoRaNetwork::_lastSendTime) < 880 && --retry>0){
        platform_debug::TracePrinter::printTrace("       [ delay ]      ("+String(retry,DEC)+")");
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(random(200,1000)));
    }
   // _mutex.lock();
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
   // _mutex.unlock();                       
}
