#include "MQTTNetwork.h"

mqtt::UserProperties MQTTNetwork::userProperties;

void MQTTNetwork::runWiFiEventService(){
     platform_debug::TracePrinter::printTrace("-----------------runWiFiEventService-----------------");
    while(true){
        osEvent evt = _mailBoxWiFiEvent.get();
        if (evt.status == osEventMail) {
          mail_wifi_event_t* event = ( mail_wifi_event_t *)evt.value.p;
          platform_debug::TracePrinter::printTrace("[WiFi-event] event: "+String(event->id,DEC));
          //std::lock_guard<rtos::Mutex> lck(_mtx);
        
          switch(event->id) {
            case SYSTEM_EVENT_STA_STOP:
                WiFi.mode(WIFI_STA);
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                platform_debug::TracePrinter::printTrace("WiFi Connecting...");
            break;
            case SYSTEM_EVENT_STA_START:
              
            break;
            case SYSTEM_EVENT_STA_GOT_IP:
                platform_debug::TracePrinter::printTrace("WiFi connected:SYSTEM_EVENT_STA_GOT_IP");
                platform_debug::TracePrinter::printTrace("IP address: "+WiFi.localIP().toString());
                platform_debug::TracePrinter::printTrace("MQTT Connecting...");
                mqttClient.connect();
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                platform_debug::TracePrinter::printTrace("WiFi lost connection"); 
                WiFi.mode(WIFI_OFF);
                break;
              case SYSTEM_EVENT_STA_CONNECTED:
                platform_debug::TracePrinter::printTrace("WiFi connected:SYSTEM_EVENT_STA_CONNECTED"); 
                break;
            default:
              platform_debug::TracePrinter::printTrace("WiFi Event Unknow"); 
            break;
          }
          _mailBoxWiFiEvent.free(event);
        }
    }
}

void MQTTNetwork::run_mail_box_topics_subscribed(){
  platform_debug::TracePrinter::printTrace("-----------------run_mail_box_topics_subscribed-----------------");
  while(true){
    osEvent evt= _mail_box_on_subscribe.get();
    if (evt.status == osEventMail) {
        mqtt::mail_subscribe_t *mail = (mqtt::mail_subscribe_t *)evt.value.p;
        for(auto& v : _onMqttConnectCallbacks){
            v.call(mail->sessionPresent);
        }
        for(auto& v : _topics){
            platform_debug::TracePrinter::printTrace("[*]subscribe: "+v);
            if(!subscribe(v, 0)){
              break;
            }
        }
        _mail_box_on_subscribe.free(mail); 
    }
  }
}

void MQTTNetwork::run_mail_box_on_message_arrived(){
  platform_debug::TracePrinter::printTrace("-----------------run_mail_box_on_message_arrived-----------------");
  while(true){
    osEvent evt= _mail_box_on_message.get();
    if (evt.status == osEventMail) {
        mail_message_t *mail = (mail_message_t *)evt.value.p;
        for(auto& v:  _onMessageCallbacks){
              v.call(mail->topic,mail->payload);
        }
        _mail_box_on_message.free(mail); 
    }
  }
}

void MQTTNetwork::WiFiEvent(system_event_id_t event, system_event_info_t info) {
  mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
  if(evt!=NULL){
    evt->id=event;
    _mailBoxWiFiEvent.put(evt);
  }else{
    platform_debug::TracePrinter::printTrace("ESP.restart()");
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(10));
    ESP.restart();
  }
}

AsyncMqttClient MQTTNetwork::mqttClient;
MQTTNetwork* MQTTNetwork::_client=nullptr;
//const char* MQTTNetwork::MQTT_HOST="mslmqtt.mic.com.cn";
const char* MQTTNetwork::MQTT_HOST="10.86.3.147";
uint16_t MQTTNetwork::MQTT_PORT = 1883;
const char* MQTTNetwork::WIFI_SSID="Mitac IOT_GPS";
const char* MQTTNetwork::WIFI_PASSWORD="6789067890";

bool MQTTNetwork::connected(){
   std::lock_guard<rtos::Mutex> lck(_mtx);
  return mqttClient.connected();
}
bool MQTTNetwork::publish(const String& topic,const String& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
   if(!connected()) {
     return false;
   }
   std::lock_guard<rtos::Mutex> lck(_mtx);
   return  mqttClient.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1;
}
bool  MQTTNetwork::subscribe(const String& topic, uint8_t qos)
{
   if(!connected()) {
     return false;
   }

   std::lock_guard<rtos::Mutex> lck(_mtx);
  return  mqttClient.subscribe(topic.c_str(), qos)!=0;;
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
    platform_debug::TracePrinter::printTrace("OK: Connected to MQTT.");
    mail_subscribe_t *mail = _mail_box_on_subscribe.alloc();
    if(mail!=NULL){
        mail->sessionPresent = sessionPresent;
        _mail_box_on_subscribe.put(mail) ;
    }
}
void MQTTNetwork::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
  platform_debug::TracePrinter::printTrace("Disconnected from MQTT.");

  #ifndef NDEBUG
  switch(reason){
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
    platform_debug::TracePrinter::printTrace("reason:TCP_DISCONNECTED");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
    platform_debug::TracePrinter::printTrace("reason:MQTT_UNACCEPTABLE_PROTOCOL_VERSION");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
    platform_debug::TracePrinter::printTrace("reason:MQTT_IDENTIFIER_REJECTED");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
    platform_debug::TracePrinter::printTrace("reason:MQTT_SERVER_UNAVAILABLE");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
    platform_debug::TracePrinter::printTrace("reason:MQTT_MALFORMED_CREDENTIALS");
    break;
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
    platform_debug::TracePrinter::printTrace("reason:MQTT_NOT_AUTHORIZED");
    break;
    case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
    platform_debug::TracePrinter::printTrace("reason:ESP8266_NOT_ENOUGH_SPAC");
    break;
    case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
    platform_debug::TracePrinter::printTrace("reason:TLS_BAD_FINGERPRINT");
    break;
  }
  #endif

      for(auto& v : _onMqttDisconnectCallbacks){
        v.call(reason);
      }
      if (_autoConnect&&WiFi.isConnected()) {
          mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
          evt->id=SYSTEM_EVENT_STA_GOT_IP;
          _mailBoxWiFiEvent.put(evt);
      }
}
    
void MQTTNetwork::onMqttSubscribe(uint16_t packetId, uint8_t qos)
 {
    platform_debug::TracePrinter::printTrace("[OK]:Subscribe acknowledged.");
    platform_debug::TracePrinter::printTrace("packetId: "+String(packetId,DEC));
    platform_debug::TracePrinter::printTrace("qos: "+String(qos,DEC));
}

void MQTTNetwork::onMqttUnsubscribe(uint16_t packetId)
{
    platform_debug::TracePrinter::printTrace("[OK]:Unsubscribe acknowledged.");
    platform_debug::TracePrinter::printTrace("packetId: "+String(packetId));
}
  
void MQTTNetwork::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
      
      mail_message_t *mail = _mail_box_on_message.alloc();
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
    platform_debug::TracePrinter::printTrace("Publish acknowledged. \n");
    platform_debug::TracePrinter::printTrace("  packetId: "+String(packetId,DEC));
}

void MQTTNetwork::init()
{
      std::lock_guard<rtos::Mutex> lck(_mtx);
     /* MQTTNetwork::MQTT_HOST    = MQTTNetwork::userProperties.host.c_str();
      MQTTNetwork::MQTT_PORT    = MQTTNetwork::userProperties.port;
      MQTTNetwork::WIFI_SSID    = MQTTNetwork::userProperties.ssid.c_str();
      MQTTNetwork::WIFI_PASSWORD= MQTTNetwork::userProperties.pass.c_str();
      */
      platform_debug::TracePrinter::printTrace("MQTTNetwork::MQTT_SSID:"+String(MQTTNetwork::WIFI_SSID));
      platform_debug::TracePrinter::printTrace("MQTTNetwork::MQTT_PASS:"+String(WIFI_PASSWORD));
      platform_debug::TracePrinter::printTrace(String(MQTT_HOST)+":"+String(MQTT_PORT,DEC));
      
    
      mqttClient.setWill("STLB_WILL",0,false,Platform::deviceInfo.BoardID.c_str(),Platform::deviceInfo.BoardID.length());
      mqttClient.setCleanSession(true);
      mqttClient.setKeepAlive(120);
      mqttClient.setClientId(Platform::deviceInfo.BoardID);
      WiFi.onEvent(std::bind(&MQTTNetwork::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2));
     // _wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)this, reinterpret_cast<TimerCallbackFunction_t>(&MQTTNetwork::_thunkConnectToWifi));
      //_mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)this, reinterpret_cast<TimerCallbackFunction_t>(&MQTTNetwork::_thunkConnectToMqtt));
  
      mqttClient.onConnect(callback(this,&MQTTNetwork::onMqttConnect));
      mqttClient.onDisconnect(callback(this,&MQTTNetwork::onMqttDisconnect));
      mqttClient.onSubscribe(callback(this,&MQTTNetwork::onMqttSubscribe));
      mqttClient.onUnsubscribe(callback(this,&MQTTNetwork::onMqttUnsubscribe));
      mqttClient.onMessage(callback(this,&MQTTNetwork::onMqttMessage));
      mqttClient.onPublish(callback(this,&MQTTNetwork::onMqttPublish));
      mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void MQTTNetwork::disconnect(bool autoConnect)
{
   std::lock_guard<rtos::Mutex> lck(_mtx);
   mqttClient.disconnect();
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
    /*
void WiFiEvent(system_event_id_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        MQTTNetwork.setMqttReconnectTimer(true);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection"); 
        // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        if(thunk_event != SYSTEM_EVENT_STA_DISCONNECTED){
            MQTTNetwork.setMqttReconnectTimer(false);
            MQTTNetwork.setWifiReconnectTimer(true);
        }
        break;
    default:break;
    }
    thunk_event = event;
}*/