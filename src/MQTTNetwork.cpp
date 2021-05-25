#include "MQTTNetwork.h"

//const char* MQTT_HOST="mslmqtt.mic.com.cn";
const char* MQTT_HOST="10.86.3.147";
uint16_t MQTT_PORT = 1883;
const char* WIFI_SSID="Mitac IOT_GPS";
const char* WIFI_PASSWORD="6789067890";

void MQTTNetwork::runWiFiEventService(){
     TracePrinter::printTrace("-----------------runWiFiEventService-----------------");
    while(true){
        osEvent evt = _mailBoxWiFiEvent.get();
        if (evt.status == osEventMail) {
          mqtt::mail_wifi_event_t* event = ( mqtt::mail_wifi_event_t *)evt.value.p;
          TracePrinter::printTrace("[WiFi-event] event: "+String(event->id,DEC));
          switch(event->id) {
            case SYSTEM_EVENT_STA_STOP:
                WiFi.mode(WIFI_STA);
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                TracePrinter::printTrace("WiFi Connecting...");
            break;
            case SYSTEM_EVENT_STA_START:
              
            break;
            case SYSTEM_EVENT_STA_GOT_IP:
                TracePrinter::printTrace("WiFi connected:SYSTEM_EVENT_STA_GOT_IP,IP address: "+WiFi.localIP().toString());
                _client.connect();
                TracePrinter::printTrace("MQTT Connecting...");
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                TracePrinter::printTrace("WiFi lost connection"); 
                WiFi.mode(WIFI_OFF);
                break;
              case SYSTEM_EVENT_STA_CONNECTED:
               // TracePrinter::printTrace("WiFi connected:SYSTEM_EVENT_STA_CONNECTED"); 
                break;
            default:
             // TracePrinter::printTrace("WiFi Event Unknow"); 
            break;
          }
          _mailBoxWiFiEvent.free(event);
        }
    }
}

void MQTTNetwork::run_mail_box_topics_subscribed(){
  TracePrinter::printTrace("-----------------run_mail_box_topics_subscribed-----------------");
  while(true){
    osEvent evt= _mail_box_on_subscribe.get();
    if (evt.status == osEventMail) {
        mqtt::mail_subscribe_t *mail = (mqtt::mail_subscribe_t *)evt.value.p;
        for(auto& v : _onMqttConnectCallbacks){
            v.call(mail->sessionPresent);
        }
        for(auto& v : _topics){
            TracePrinter::printTrace("[*]subscribe: "+v);
            if(!subscribe(v, 0)){
              break;
            }
        }
        _mail_box_on_subscribe.free(mail); 
    }
  }
}

void MQTTNetwork::run_mail_box_on_message_arrived(){
  TracePrinter::printTrace("----- run_mail_box_on_message_arrived --------");
  while(true){
    osEvent evt= _mail_box_on_message.get();
    if (evt.status == osEventMail) {
        mqtt::mail_mqtt_t *mail = (mqtt::mail_mqtt_t *)evt.value.p;
        for(auto& v:  _onMessageCallbacks){
              v.call(mail->topic,mail->payload);
        }
        _mail_box_on_message.free(mail); 
    }
  }
}

void MQTTNetwork::WiFiEvent(system_event_id_t event, system_event_info_t info) {

  mqtt::mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
  if(evt!=NULL){
    evt->id=event;
    _mailBoxWiFiEvent.put(evt);
  }
}

void MQTTNetwork::init()
{
      std::lock_guard<rtos::Mutex> lck(_mtx);
      MQTT_HOST    = Platform::getUserProperties().host.c_str();
      MQTT_PORT    = Platform::getUserProperties().port;
      WIFI_SSID    = Platform::getUserProperties().ssid.c_str();
      WIFI_PASSWORD= Platform::getUserProperties().pass.c_str();
      
      TracePrinter::printf("AsyncMqttClient:size:%d\n",sizeof(AsyncMqttClient));
      TracePrinter::printf("_threadWiFiEvent:%p\n",&_threadWiFiEvent);
      TracePrinter::printf("_threadOnMessage:%p\n",&_threadOnMessage);
      TracePrinter::printf("_threadSubscribe:%p\n",&_threadSubscribe);
      TracePrinter::printTrace(String(MQTT_HOST)+":"+String(MQTT_PORT,DEC));
      
      _client.setWill("STLB_WILL",0,false,Platform::getDeviceInfo().BoardID.c_str(),Platform::getDeviceInfo().BoardID.length());
      _client.setCleanSession(true);
      _client.setKeepAlive(120);
      _client.setClientId(Platform::getDeviceInfo().BoardID);
      WiFi.onEvent(_wifiCB);

      _client.onConnect(callback(this,&MQTTNetwork::onMqttConnect));
      _client.onDisconnect(callback(this,&MQTTNetwork::onMqttDisconnect));
      _client.onSubscribe(callback(this,&MQTTNetwork::onMqttSubscribe));
      _client.onUnsubscribe(callback(this,&MQTTNetwork::onMqttUnsubscribe));
      _client.onMessage(callback(this,&MQTTNetwork::onMqttMessage));
      _client.onPublish(callback(this,&MQTTNetwork::onMqttPublish));
      _client.setServer(MQTT_HOST, MQTT_PORT);

      WiFi.setAutoReconnect(false);
      WiFi.setAutoConnect(false);
      
}

 void MQTTNetwork::startup()  //throw (os::thread_error)
{
        osStatus status =  _threadWiFiEvent.start(callback(this,&MQTTNetwork::runWiFiEventService));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadWiFiEvent.get_name()):NULL);
        status =  _threadOnMessage.start(callback(this,&MQTTNetwork::run_mail_box_on_message_arrived));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadOnMessage.get_name()):NULL);
        status =   _threadSubscribe.start(callback(this,&MQTTNetwork::run_mail_box_topics_subscribed));
        (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadSubscribe.get_name()):NULL);
        
        mqtt::mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
        evt->id=SYSTEM_EVENT_STA_STOP;
        _mailBoxWiFiEvent.put(evt);

}

bool MQTTNetwork::connected(){
   std::lock_guard<rtos::Mutex> lck(_mtx);
  return _client.connected();
}
bool MQTTNetwork::publish(const String& topic,const String& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
   if(!connected()) {
     return false;
   }
   std::lock_guard<rtos::Mutex> lck(_mtx);
   return _client.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1;
}
bool  MQTTNetwork::subscribe(const String& topic, uint8_t qos)
{
   if(!connected()) {
     return false;
   }

   std::lock_guard<rtos::Mutex> lck(_mtx);
  return _client.subscribe(topic.c_str(), qos)!=0;;
}
bool MQTTNetwork::unsubscribe(const String& topic)
{ 
   
   if(!connected()) {
     return false;
   }
    std::lock_guard<rtos::Mutex> lck(_mtx);
   return unsubscribe(topic.c_str());
}
void MQTTNetwork::onMqttConnect(bool sessionPresent)
{
    TracePrinter::printTrace("OK: Connected to MQTT.");
    mqtt::mail_subscribe_t *mail = _mail_box_on_subscribe.alloc();
    if(mail!=NULL){
        mail->sessionPresent = sessionPresent;
        _mail_box_on_subscribe.put(mail) ;
    }
}
void MQTTNetwork::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
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
      if (_autoConnect&&WiFi.isConnected()) {
          mqtt::mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
          evt->id=SYSTEM_EVENT_STA_GOT_IP;
          _mailBoxWiFiEvent.put(evt);
      }
}
    
void MQTTNetwork::onMqttSubscribe(uint16_t packetId, uint8_t qos)
 {
    TracePrinter::printTrace("[OK]:Subscribe acknowledged.");
    TracePrinter::printTrace("packetId: "+String(packetId,DEC));
    TracePrinter::printTrace("qos: "+String(qos,DEC));
}

void MQTTNetwork::onMqttUnsubscribe(uint16_t packetId)
{
    TracePrinter::printTrace("[OK]:Unsubscribe acknowledged.");
    TracePrinter::printTrace("packetId: "+String(packetId));
}
  
void MQTTNetwork::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
      
      mqtt::mail_mqtt_t *mail = _mail_box_on_message.alloc();
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
  
void MQTTNetwork::onMqttPublish(uint16_t packetId) 
{
    TracePrinter::printTrace("Publish acknowledged. \n");
    TracePrinter::printTrace("  packetId: "+String(packetId,DEC));
}

void MQTTNetwork::disconnect(bool autoConnect)
{
   std::lock_guard<rtos::Mutex> lck(_mtx);
  _client.disconnect();
}

void MQTTNetwork::addOnMessageCallback(Callback<void(const String&,const String&)> func)
{
    _onMessageCallbacks.push_back(func);
}
void MQTTNetwork::addSubscribeTopic(const String& topic,int qos)
{
    _topics.insert(topic);
}
void MQTTNetwork::addSubscribeTopics(std::vector<String>& vec)
{
  for(auto& v : vec){
    _topics.insert(v);
  }
}
void MQTTNetwork::addOnMqttConnectCallback(Callback<void(bool)> func)
{
   _onMqttConnectCallbacks.push_back(func);
}
void MQTTNetwork::addOnMqttDisonnectCallback(Callback<void(AsyncMqttClientDisconnectReason)> func)
{
   _onMqttDisconnectCallbacks.push_back(func);
}
