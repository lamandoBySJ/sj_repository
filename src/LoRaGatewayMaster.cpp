#include "LoRaGatewayMaster.h"

void LoRaGatewayMaster::startup()
{
      _thread.start(callback(this,&LoRaGatewayMaster::run));
}
void LoRaGatewayMaster::run()
{
    DynamicJsonDocument  doc(1024);
    while(true){
        osEvent evt= _mail_box.get();
        if (evt.status == osEventMail) {
            network::mail_t *mail = (mail_t *)evt.value.p;
            DeserializationError error = deserializeJson(doc,mail->payload);
            if (!error)
            { 
                if (doc.containsKey("beacon")) {
                    Serial.println(doc["beacon"].as<String>());
                } 
            }
            _mail_box.free(mail); 
        }
    }
}

void LoRaGatewayMaster::onMessageCallback(const String& topic,const String& payload)
{
      network::mail_t *mail =  _mail_box.alloc();
      if(mail!=NULL){
        mail->topic = String(topic);
        mail->payload = String(payload);
        _mail_box.put(mail) ;
      }
    }
void LoRaGatewayMaster::attach(Callback<void(const String&,const String&)> func)
{
        #if !defined(NDEBUG)
        _debugTraceCallbacks.push_back(func);
        #endif
}