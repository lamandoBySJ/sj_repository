#include "MQTTNetwork.h"

void MQTTNetwork::runWiFiEventService(){
    for(;;){
      osEvent evt = _mailBoxWiFiEvent.get();
      if (evt.status == osEventMail) {
        mail_wifi_event_t* event = ( mail_wifi_event_t *)evt.value.p;
        platform_debug::TracePrinter::printTrace("[WiFi-event] event: "+String(event->id,DEC));
        switch(event->id) {
          case SYSTEM_EVENT_STA_STOP:
              WiFi.mode(WIFI_STA);
              WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
              //std::this_thread::sleep_for(std::chrono::seconds(1));
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

void MQTTNetwork::WiFiEvent(system_event_id_t event, system_event_info_t info) {
  mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
  if(evt!=NULL){
    evt->id=event;
    _mailBoxWiFiEvent.put(evt);
  }else{
    platform_debug::TracePrinter::printTrace("ESP.restart()");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    ESP.restart();
  }
}

AsyncMqttClient MQTTNetwork::mqttClient;
///*
//const char* MQTTNetwork::MQTT_HOST="mslmqtt.mic.com.cn";
const char* MQTTNetwork::MQTT_HOST="mslmqtt.mic.com.cn";
uint16_t MQTTNetwork::MQTT_PORT = 1883;
const char* MQTTNetwork::WIFI_SSID="IoTwlan";
const char* MQTTNetwork::WIFI_PASSWORD="mitac1993";
//const char* MQTTNetwork::WIFI_SSID="IPS";
//const char* MQTTNetwork::WIFI_PASSWORD="Aa000000";
//*/
/*
const char* MQTTNetwork::MQTT_HOST("192.168.1.133");
uint16_t MQTTNetwork::MQTT_PORT =1883;
const char* MQTTNetwork::WIFI_SSID="IPS";
const char* MQTTNetwork::WIFI_PASSWORD="Aa000000";
//*/

/*
void MQTTNetwork::_thunkConnectToWifi(void* pvTimerID)
{
    static_cast<MQTTNetwork*>(pvTimerID)->_connectToWifi();
}
void MQTTNetwork::_thunkConnectToMqtt(void* pvTimerID) 
{
    static_cast<MQTTNetwork*>(pvTimerID)-> _connectToMqtt();
}
void MQTTNetwork::_connectToMqtt()
{
    //std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
    //lck.lock();
    platform_debug::TracePrinter::printTrace("MQTT Connecting...");
    mqttClient.connect();
}
void MQTTNetwork::_connectToWifi() 
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}*/
bool MQTTNetwork::connected(){
  std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
  lck.lock();
  return mqttClient.connected();
}
bool MQTTNetwork::publish(const String& topic,const String& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
  
   if(!connected()) {
     return false;
   }
    std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
    lck.lock();
   return  mqttClient.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1;
}
bool  MQTTNetwork::subscribe(const String& topic, uint8_t qos)
{
   if(!connected()) {
     return false;
   }

  std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
  lck.lock();
  return  mqttClient.subscribe(topic.c_str(), qos)!=0;;
}
bool MQTTNetwork::unsubscribe(const String& topic)
{ 
   
   if(!connected()) {
     return false;
   }
   std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
   lck.lock();
   return unsubscribe(topic.c_str());
}
void MQTTNetwork::onMqttConnect(bool sessionPresent)
{
    platform_debug::TracePrinter::printTrace("OK: Connected to MQTT.");
    mail_on_connect_t *mail = _mail_box_subscribe.alloc();
    if(mail!=NULL){
        mail->sessionPresent = sessionPresent;
        _mail_box_subscribe.put(mail) ;
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

      std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
      lck.lock();
      if (_autoConnect&&WiFi.isConnected()) {
        lck.unlock();
       // setMqttReconnectTimer(true);
        platform_debug::TracePrinter::printTrace("[ setMqttReconnectTimer(true) ]");
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
      
      mail_t *mail = _mail_box.alloc();
      if(mail!=NULL){
        mail->topic = topic;
        mail->payload = String(payload).substring(0,len);
        mail->properties.qos = properties.qos;
        mail->properties.dup = properties.dup;
        mail->properties.retain = properties.retain;
        mail->index = index;
        mail->total = total;
        _mail_box.put(mail) ;
      }
}
  
void MQTTNetwork::onMqttPublish(uint16_t packetId) 
{
    platform_debug::TracePrinter::printTrace("Publish acknowledged. \n");
    platform_debug::TracePrinter::printTrace("  packetId: "+String(packetId,DEC));
}
/*
void MQTTNetwork::setMqttReconnectTimer(bool start)
{
    std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
    lck.lock();
    if(start){
          xTimerStart(_mqttReconnectTimer,0);
    }else{
          xTimerStop(_mqttReconnectTimer,0);
    }
}

void MQTTNetwork::setWifiReconnectTimer(bool start)
{
    std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
    lck.lock();
      if(start){
          xTimerStart(_wifiReconnectTimer,0);
      }else{
          xTimerStop(_wifiReconnectTimer,0);
      }
}
*/
void MQTTNetwork::startup(){

      MQTTNetwork::MQTT_HOST    = user_properties::host.c_str();
      MQTTNetwork::MQTT_PORT    = user_properties::port;
      MQTTNetwork::WIFI_SSID    = user_properties::ssid.c_str();
      MQTTNetwork::WIFI_PASSWORD= user_properties::pass.c_str();
      platform_debug::TracePrinter::printTrace("MQTTNetwork::MQTT_SSID:"+String(MQTTNetwork::WIFI_SSID));
      platform_debug::TracePrinter::printTrace("MQTTNetwork::MQTT_PASS:"+String(WIFI_PASSWORD));
      platform_debug::TracePrinter::printTrace(String(MQTT_HOST)+":"+String(MQTT_PORT,DEC));
      
    
      mqttClient.setWill("STLB_WILL",0,false,platform_debug::DeviceInfo::BoardID.c_str(),platform_debug::DeviceInfo::BoardID.length());
      mqttClient.setCleanSession(true);
      mqttClient.setKeepAlive(120);
      mqttClient.setClientId(platform_debug::DeviceInfo::BoardID);
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
      
      _threadOnMessage = std::thread(&MQTTNetwork::run_mail_box_message,this);
     //_threadOnMessage.start(callback(this,&MQTTNetwork::run_mail_box));
     _threadSubscribe.start(callback(this,&MQTTNetwork::run_mail_box_subscribe));
     _threadWiFiEvent.start(callback(this,&MQTTNetwork::runWiFiEventService));

    mail_wifi_event_t* evt=_mailBoxWiFiEvent.alloc();
    evt->id=SYSTEM_EVENT_STA_STOP;
    _mailBoxWiFiEvent.put(evt);
}

void MQTTNetwork::disconnect(bool autoConnect)
{
  std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
  lck.lock();
  mqttClient.disconnect();
}

void MQTTNetwork::run_mail_box_message(){
  while(true){
   
    osEvent evt= _mail_box.get();
    if (evt.status == osEventMail) {
        mail_t *mail = (mail_t *)evt.value.p;
        for(auto& v:  _onMessageCallbacks){
              v.call(mail->topic,mail->payload);
        }
        
        _mail_box.free(mail); 
    }
  }
 // vTaskDelete(NULL);
}

void MQTTNetwork::run_mail_box_subscribe(){
  while(true){
   
    osEvent evt= _mail_box_subscribe.get();
    if (evt.status == osEventMail) {
        
        mqtt::mail_on_connect_t *mail = (mqtt::mail_on_connect_t *)evt.value.p;
        for(auto& v : _onMqttConnectCallbacks){
            v.call(mail->sessionPresent);
        }
        for(auto& v : _topics){
            platform_debug::TracePrinter::printTrace("[*]subscribe: "+v);
            if(!subscribe(v, 0)){
              break;
            }
        }
        _mail_box_subscribe.free(mail); 
    }
  }
 // vTaskDelete(NULL);
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