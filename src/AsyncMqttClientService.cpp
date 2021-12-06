#include "AsyncMqttClientService.h"

void AsyncMqttClientService::task_on_message_service(){
  while(true){
    osEvent evt= _mail_box_on_message.get();
    if (evt.status == osEventMail) {
        mqtt::mail_on_message_t *mail = (mqtt::mail_on_message_t *)evt.value.p;
        for(auto& v:  _onMqttMessageCallbacks){
              v.call(mail->topic,mail->payload);
        }
        _mail_box_on_message.free(mail); 
    }
  }
}

void AsyncMqttClientService::task_on_connect_service()
{
  while(true){
      osEvent evt= _mail_box_on_connect.get();
      if (evt.status == osEventMail) {
          mqtt::mail_on_connect_t *mail = (mqtt::mail_on_connect_t *)evt.value.p;
          for(auto& v : _onMqttConnectCallbacks){
              v.call(mail->sessionPresent);
          }
          _mail_box_on_connect.free(mail); 
      }
    }
}

void AsyncMqttClientService::task_on_subscribe_service(){
  while(true){
    osEvent evt= _mail_box_on_subscribe.get();
    if (evt.status == osEventMail) {
        mqtt::mail_on_subscribe_t *mail = (mqtt::mail_on_subscribe_t *)evt.value.p;
        TracePrinter::printTrace("[OK]:Subscribe acknowledged.");
        TracePrinter::printTrace("packetId: "+String(mail->packetId,DEC));
        TracePrinter::printTrace("qos: "+String(mail->qos,DEC));
        for(auto& v : _onMqttSubscribeCallbacks){
            v.call(mail->packetId,mail->qos);
        }
        _mail_box_on_subscribe.free(mail); 
    }
  }
}
void AsyncMqttClientService::task_on_unsubscribe_service(){
  while(true){
    osEvent evt= _mail_box_on_unsubscribe.get();
    if (evt.status == osEventMail) {
        mqtt::mail_on_unsubscribe_t *mail = (mqtt::mail_on_unsubscribe_t *)evt.value.p;
        TracePrinter::printTrace("[OK]:Unsubscribe acknowledged.");
        TracePrinter::printTrace("packetId: "+String(mail->packetId,DEC));
        for(auto& v : _onMqttUnsubscribeCallbacks){
            v.call(mail->packetId);
        }
        _mail_box_on_unsubscribe.free(mail); 
    }
  }
}

const char* MQTT_HOST="mslmqtt-test.mic.com.cn";
uint16_t MQTT_PORT = 1883;

void AsyncMqttClientService::init()
{
      std::lock_guard<rtos::Mutex> lck(_mtx);
      MQTT_HOST    = platformio_api::get_user_properties().host.c_str();
      MQTT_PORT    = platformio_api::get_user_properties().port;
      
      TracePrinter::printTrace(String(MQTT_HOST)+":"+String(MQTT_PORT,DEC));
      
      _client.setWill("STLB_WILL",0,false,platformio_api::get_device_info().BoardID.c_str(),platformio_api::get_device_info().BoardID.length());
      _client.setCleanSession(true);
      _client.setKeepAlive(120);
      _client.setClientId(platformio_api::get_device_info().BoardID);
    

      _client.onConnect(mbed::callback(this,&AsyncMqttClientService::onMqttConnect));
      _client.onDisconnect(mbed::callback(this,&AsyncMqttClientService::onMqttDisconnect));
      _client.onSubscribe(mbed::callback(this,&AsyncMqttClientService::onMqttSubscribe));
      _client.onUnsubscribe(mbed::callback(this,&AsyncMqttClientService::onMqttUnsubscribe));
      _client.onMessage(mbed::callback(this,&AsyncMqttClientService::onMqttMessage));
      _client.onPublish(mbed::callback(this,&AsyncMqttClientService::onMqttPublish));
      _client.setServer(MQTT_HOST, MQTT_PORT);  
}

 void AsyncMqttClientService::startup()  //throw (os::thread_error)
{   
        init();
        
        osStatus status =  _threadOnMessage.start(mbed::callback(this,&AsyncMqttClientService::task_on_message_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnMessage.get_name()):NULL);

         status =   _threadOnSubscribe.start(mbed::callback(this,&AsyncMqttClientService::task_on_subscribe_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnSubscribe.get_name()):NULL);

         status =   _threadOnUnsubscribe.start(mbed::callback(this,&AsyncMqttClientService::task_on_unsubscribe_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnUnsubscribe.get_name()):NULL);
       
        status =   _threadOnConnect.start(mbed::callback(this,&AsyncMqttClientService::task_on_connect_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnConnect.get_name()):NULL);

       
}

bool AsyncMqttClientService::connected(){
   std::lock_guard<rtos::Mutex> lck(_mtx);
  return _client.connected();
}
bool AsyncMqttClientService::publish(const String& topic,const String& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
   return _client.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1;
}
bool  AsyncMqttClientService::subscribe(const String& topic, uint8_t qos)
{
   std::lock_guard<rtos::Mutex> lck(_mtx);
  return _client.subscribe(topic.c_str(), qos)!=0;;
}
bool AsyncMqttClientService::unsubscribe(const String& topic)
{ 
  std::lock_guard<rtos::Mutex> lck(_mtx);
   return unsubscribe(topic.c_str());
}

void AsyncMqttClientService::onMqttConnect(bool sessionPresent)
{
    TracePrinter::printTrace("OK: Connected to MQTT.");
    mqtt::mail_on_connect_t *mail = _mail_box_on_connect.alloc();
    if(mail!=NULL){
        mail->sessionPresent = sessionPresent;
        _mail_box_on_connect.put(mail) ;
    }
}
void AsyncMqttClientService::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
  TracePrinter::printTrace("Disconnected from MQTT."+String((int)reason,DEC));
  for(auto& v : _onMqttDisconnectCallbacks){
        v.call(reason);
  }
}
    
void AsyncMqttClientService::onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    mqtt::mail_on_subscribe_t* evt=_mail_box_on_subscribe.alloc();
    evt->packetId=packetId;
    evt->qos=qos;
    _mail_box_on_subscribe.put(evt);
}

void AsyncMqttClientService::onMqttUnsubscribe(uint16_t packetId)
{
    TracePrinter::printTrace("[OK]:Unsubscribe acknowledged.");
    TracePrinter::printTrace("packetId: "+String(packetId));
    mqtt::mail_on_unsubscribe_t* evt=_mail_box_on_unsubscribe.alloc();
    evt->packetId=packetId;
    _mail_box_on_unsubscribe.put(evt);
}

void AsyncMqttClientService::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
      mqtt::mail_on_message_t *mail = _mail_box_on_message.alloc();
      if(mail!=NULL){
        mail->topic = topic;
        mail->payload = String(payload).substring(0,len);
        mail->properties.qos = properties.qos;
        mail->properties.dup = properties.dup;
        mail->properties.retain = properties.retain;
        mail->index = index;
        mail->total = total;
        _mail_box_on_message.put(mail) ;
      }
}
  
void AsyncMqttClientService::onMqttPublish(uint16_t packetId) 
{
    TracePrinter::printTrace("Publish acknowledged. \n");
    TracePrinter::printTrace("  packetId: "+String(packetId,DEC));
}

void AsyncMqttClientService::connect()
{
   std::lock_guard<rtos::Mutex> lck(_mtx);
  _client.connect();
}
void AsyncMqttClientService::disconnect()
{
   std::lock_guard<rtos::Mutex> lck(_mtx);
  _client.disconnect();
}
void AsyncMqttClientService::addOnMqttMessageCallback(mbed::Callback<void(const String&,const String&)> func)
{
    _onMqttMessageCallbacks.push_back(func);
}
void AsyncMqttClientService::addOnMqttConnectCallback(mbed::Callback<void(bool)> func)
{
   _onMqttConnectCallbacks.push_back(func);
}
void AsyncMqttClientService::addOnMqttSubscribeCallback(mbed::Callback<void(uint16_t, uint8_t)> func)
{
   _onMqttSubscribeCallbacks.push_back(func);
}
void AsyncMqttClientService::addOnMqttUnsubscribeCallback(mbed::Callback<void(uint16_t)> func)
{
   _onMqttUnsubscribeCallbacks.push_back(func);  
}
void AsyncMqttClientService::addOnMqttDisonnectCallback(mbed::Callback<void(AsyncMqttClientDisconnectReason)> func)
{
   _onMqttDisconnectCallbacks.push_back(func);
}

