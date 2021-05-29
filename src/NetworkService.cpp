#include "NetworkService.h"
#include "LEDIndicator.h"

//const char* MQTT_HOST="mslmqtt.mic.com.cn";
const char* MQTT_HOST="10.86.3.147";
uint16_t MQTT_PORT = 1883;
const char* WIFI_SSID="Mitac IOT_GPS";
const char* WIFI_PASSWORD="6789067890";
void NetworkService::switchToSoftAP(){
    //std::lock_guard<rtos::Mutex> lck(_mtx);
    _autoConnect=false;
    if(_client.connected()){
         _client.disconnect();
    }
    _threadOnMessage.terminate();
    _threadOnSubscribe.terminate();
    _threadOnUnsubscribe.terminate();
    _threadOnConnect.terminate();
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_AP_STA);
    WiFi.enableAP(true);
    WiFi.begin();
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    for(auto& call:_onWiFiModeCallbacks){
      call.call();
    }
    _threadWiFiEvent.terminate();
}
void NetworkService::set_system_event(system_event_id_t id,uint32_t signal_id){
    mqtt::mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
    evt->id=id;
    evt->signal_id = signal_id;
    _mailBoxWiFiEvent.put(evt);
}
void NetworkService::task_system_event_service(){
    while(true){
        osEvent evt = _mailBoxWiFiEvent.get();
        if (evt.status == osEventMail) {
          mqtt::mail_wifi_event_t* event = ( mqtt::mail_wifi_event_t *)evt.value.p;
          TracePrinter::printTrace("[WiFi-event] event: "+String(event->id,DEC));
          switch(event->id) {
            case SYSTEM_EVENT_STA_STOP:
                WiFi.mode(WIFI_STA);
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD,_wifi_channel);
                TracePrinter::printTrace("WiFi Connecting...");
            break;
            case SYSTEM_EVENT_STA_START:
              
            break;
            case SYSTEM_EVENT_STA_GOT_IP:
                TracePrinter::printTrace("WiFi connected:SYSTEM_EVENT_STA_GOT_IP,IP address: "+WiFi.localIP().toString());
                LEDIndicator::getLEDIndicator().io_state_wifi(true);
                if(!_client.connected()&&_autoConnect){
                    _client.connect();
                }
                TracePrinter::printTrace("MQTT Connecting...");
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                TracePrinter::printTrace("WiFi lost connection"); 
                LEDIndicator::getLEDIndicator().io_state_wifi(false);
                WiFi.mode(WIFI_OFF);
                break;
              case SYSTEM_EVENT_STA_CONNECTED:
               // TracePrinter::printTrace("WiFi connected:SYSTEM_EVENT_STA_CONNECTED"); 
                break;
            case SYSTEM_EVENT_MAX:
                if(event->signal_id==99){
                  switchToSoftAP();
                }
                break;
            default:
             // TracePrinter::printTrace("WiFi Event Unknow"); 
            break;
          }
          _mailBoxWiFiEvent.free(event);
        }
    }
}

void NetworkService::task_on_message_service(){
  while(true){
    osEvent evt= _mail_box_on_message.get();
    if (evt.status == osEventMail) {
        mqtt::mail_on_message_t *mail = (mqtt::mail_on_message_t *)evt.value.p;
        for(auto& v:  _onMessageCallbacks){
              v.call(mail->topic,mail->payload);
        }
        _mail_box_on_message.free(mail); 
    }
  }
}

void NetworkService::task_on_connect_service()
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

void NetworkService::task_on_subscribe_service(){
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
void NetworkService::task_on_unsubscribe_service(){
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

void NetworkService::WiFiEvent(system_event_id_t event, system_event_info_t info) {

  mqtt::mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
  if(evt!=NULL){
    evt->id=event;
    _mailBoxWiFiEvent.put(evt);
  }
}

void NetworkService::init()
{
      std::lock_guard<rtos::Mutex> lck(_mtx);
      MQTT_HOST    = platformio_api::get_user_properties().host.c_str();
      MQTT_PORT    = platformio_api::get_user_properties().port;
      WIFI_SSID    = platformio_api::get_user_properties().ssid.c_str();
      WIFI_PASSWORD= platformio_api::get_user_properties().pass.c_str();

      TracePrinter::printTrace(String(MQTT_HOST)+":"+String(MQTT_PORT,DEC));
      
      _client.setWill("STLB_WILL",0,false,platformio_api::get_device_info().BoardID.c_str(),platformio_api::get_device_info().BoardID.length());
      _client.setCleanSession(true);
      _client.setKeepAlive(120);
      _client.setClientId(platformio_api::get_device_info().BoardID);
      WiFi.onEvent(_wifiCB);

      _client.onConnect(callback(this,&NetworkService::onMqttConnect));
      _client.onDisconnect(callback(this,&NetworkService::onMqttDisconnect));
      _client.onSubscribe(callback(this,&NetworkService::onMqttSubscribe));
      _client.onUnsubscribe(callback(this,&NetworkService::onMqttUnsubscribe));
      _client.onMessage(callback(this,&NetworkService::onMqttMessage));
      _client.onPublish(callback(this,&NetworkService::onMqttPublish));
      _client.setServer(MQTT_HOST, MQTT_PORT);

      WiFi.setAutoReconnect(false);
      WiFi.setAutoConnect(false);
      
}

 void NetworkService::startup()  //throw (os::thread_error)
{
        osStatus status =  _threadWiFiEvent.start(callback(this,&NetworkService::task_system_event_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadWiFiEvent.get_name()):NULL);
        
        status =  _threadOnMessage.start(callback(this,&NetworkService::task_on_message_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnMessage.get_name()):NULL);

         status =   _threadOnSubscribe.start(callback(this,&NetworkService::task_on_subscribe_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnSubscribe.get_name()):NULL);

         status =   _threadOnUnsubscribe.start(callback(this,&NetworkService::task_on_unsubscribe_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnUnsubscribe.get_name()):NULL);
       
        status =   _threadOnConnect.start(callback(this,&NetworkService::task_on_connect_service));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnConnect.get_name()):NULL);

        set_system_event(SYSTEM_EVENT_STA_STOP);
}
void NetworkService::terminate()
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
     _autoConnect=false;
    if(_client.connected()){
       _client.disconnect();
    }
    _threadWiFiEvent.terminate();
    _threadOnMessage.terminate();
    _threadOnSubscribe.terminate();
    _threadOnUnsubscribe.terminate();
    _threadOnConnect.terminate();
}
bool NetworkService::connected(){
   std::lock_guard<rtos::Mutex> lck(_mtx);
  return _client.connected();
}
bool NetworkService::publish(const String& topic,const String& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
   if(!_client.connected()) {
     return false;
   }
   return _client.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1;
}
bool  NetworkService::subscribe(const String& topic, uint8_t qos)
{
   if(!connected()) {
     return false;
   }

   std::lock_guard<rtos::Mutex> lck(_mtx);
  return _client.subscribe(topic.c_str(), qos)!=0;;
}
bool NetworkService::unsubscribe(const String& topic)
{ 
   
   if(!connected()) {
     return false;
   }
    std::lock_guard<rtos::Mutex> lck(_mtx);
   return unsubscribe(topic.c_str());
}



void NetworkService::onMqttConnect(bool sessionPresent)
{
    TracePrinter::printTrace("OK: Connected to MQTT.");
    mqtt::mail_on_connect_t *mail = _mail_box_on_connect.alloc();
    if(mail!=NULL){
        mail->sessionPresent = sessionPresent;
        _mail_box_on_connect.put(mail) ;
    }
}
void NetworkService::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
  TracePrinter::printTrace("Disconnected from MQTT.");

  #ifndef NDEBUG
  switch(reason){
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
    TracePrinter::printTrace("reason:TCP_DISCONNECTED");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
    TracePrinter::printTrace("reason:MQTT_UNACCEPTABLE_PROTOCOL_VERSION");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
    TracePrinter::printTrace("reason:MQTT_IDENTIFIER_REJECTED");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
    TracePrinter::printTrace("reason:MQTT_SERVER_UNAVAILABLE");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
    TracePrinter::printTrace("reason:MQTT_MALFORMED_CREDENTIALS");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
    TracePrinter::printTrace("reason:MQTT_NOT_AUTHORIZED");
    break;
    case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
    TracePrinter::printTrace("reason:ESP8266_NOT_ENOUGH_SPAC");
    break;
    case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
    TracePrinter::printTrace("reason:TLS_BAD_FINGERPRINT");
    break;
  }
  #endif

  for(auto& v : _onMqttDisconnectCallbacks){
        v.call(reason);
  }

  if (WiFi.isConnected()) {
      set_system_event(SYSTEM_EVENT_STA_GOT_IP);
  }
}
    
void NetworkService::onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    mqtt::mail_on_subscribe_t* evt=_mail_box_on_subscribe.alloc();
    evt->packetId=packetId;
    evt->qos=qos;
    _mail_box_on_subscribe.put(evt);
}

void NetworkService::onMqttUnsubscribe(uint16_t packetId)
{
    TracePrinter::printTrace("[OK]:Unsubscribe acknowledged.");
    TracePrinter::printTrace("packetId: "+String(packetId));
    mqtt::mail_on_unsubscribe_t* evt=_mail_box_on_unsubscribe.alloc();
    evt->packetId=packetId;
    _mail_box_on_unsubscribe.put(evt);
}

void NetworkService::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
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
  
void NetworkService::onMqttPublish(uint16_t packetId) 
{
    TracePrinter::printTrace("Publish acknowledged. \n");
    TracePrinter::printTrace("  packetId: "+String(packetId,DEC));
}

void NetworkService::connect()
{
   std::lock_guard<rtos::Mutex> lck(_mtx);
   _autoConnect=true;
  _client.connect();
}
void NetworkService::disconnect(bool autoConnect)
{
   std::lock_guard<rtos::Mutex> lck(_mtx);
   _autoConnect=autoConnect;
  _client.disconnect();
}
void NetworkService::addOnMqttMessageCallback(mbed::Callback<void(const String&,const String&)> func)
{
    _onMessageCallbacks.push_back(func);
}
void NetworkService::addOnMqttConnectCallback(mbed::Callback<void(bool)> func)
{
   _onMqttConnectCallbacks.push_back(func);
}
void NetworkService::addOnMqttSubscribeCallback(mbed::Callback<void(uint16_t, uint8_t)> func)
{
   _onMqttSubscribeCallbacks.push_back(func);
}
void NetworkService::addOnMqttUnsubscribeCallback(mbed::Callback<void(uint16_t)> func)
{
   _onMqttUnsubscribeCallbacks.push_back(func);  
}
void NetworkService::addOnMqttDisonnectCallback(mbed::Callback<void(AsyncMqttClientDisconnectReason)> func)
{
   _onMqttDisconnectCallbacks.push_back(func);
}
void NetworkService::addOnWiFiModeCallback(mbed::Callback<void(void)> func)
{
   _onWiFiModeCallbacks.push_back(func);
}
