#include "../NetworkEngine/NetworkEngine.h"
    /*
void WiFiEvent(system_event_id_t event) {

    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        networkEngine.setMqttReconnectTimer(true);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection"); 
        // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        if(thunk_event != SYSTEM_EVENT_STA_DISCONNECTED){
            networkEngine.setMqttReconnectTimer(false);
            networkEngine.setWifiReconnectTimer(true);
        }
        break;
    default:break;
    }
    thunk_event = event;
}*/

void NetworkEngine::WiFiEvent(system_event_id_t event, system_event_info_t info) {
 
    printTrace("[WiFi-event] event: "+String(event,DEC));
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        printTrace("WiFi connected");
        printTrace("IP address: "+WiFi.localIP().toString());
        setMqttReconnectTimer(true);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        printTrace("WiFi lost connection"); 
        // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        if(_event != SYSTEM_EVENT_STA_DISCONNECTED){
            setMqttReconnectTimer(false);
            setWifiReconnectTimer(true);
        }
        break;
    default:break;
    }
    _event = event;
}

AsyncMqttClient NetworkEngine::mqttClient;
rtos::Mutex NetworkEngine::_mutex;
rtos::Mutex NetworkEngine::std_mutex;
IPAddress NetworkEngine::MQTT_HOST(192, 168, 1, 133);
uint16_t NetworkEngine::MQTT_PORT =1883;
const char* NetworkEngine::WIFI_SSID="IPS";
const char* NetworkEngine::WIFI_PASSWORD="Aa000000";

void NetworkEngine::_thunkConnectToWifi(void* pvTimerID)
{
    // printTrace("_thunkConnectToWiFi...");
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
     static_cast<NetworkEngine*>(pvTimerID)->_connectToWifi();
}
void NetworkEngine::_thunkConnectToMqtt(void* pvTimerID) 
{
     //printTrace("_thunkConnectToMQTT... \n");
      static_cast<NetworkEngine*>(pvTimerID)-> _connectToMqtt();
}
void NetworkEngine::_connectToMqtt()
{
       // printTrace("Connecting to MQTT...");
        // _mutex.lock();
        mqttClient.connect();
       // _mutex.unlock();
}
void NetworkEngine::_connectToWifi() 
{
       //printTrace("Connecting to Wi-Fi...");
       // _mutex.lock();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
       // _mutex.unlock();
}

bool NetworkEngine::publish(String& topic,String& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
     _mutex.lock();
     if(!_connected){
        _mutex.unlock();
        return false;
     }
   _mutex.unlock();
   return  mqttClient.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1;
}
bool NetworkEngine::publish(String& topic,const String&& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
    _mutex.lock();
     if(!_connected){
        _mutex.unlock();
        return false;
     }
   _mutex.unlock();
   return  mqttClient.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1;
}
bool NetworkEngine::publish(const String&& topic,String& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
    _mutex.lock();
     if(!_connected){
        _mutex.unlock();
        return false;
     }
   _mutex.unlock();
   return  mqttClient.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1;
}
bool NetworkEngine::publish(const String&& topic,const String&& payload, uint8_t qos, bool retain,bool dup, uint16_t message_id)
{
     _mutex.lock();
     if(!_connected){
        _mutex.unlock();
        return false;
     }
   _mutex.unlock();
   return  mqttClient.publish(topic.c_str(),qos, retain, payload.c_str(),payload.length(),dup,message_id) == 1 ;
}

bool  NetworkEngine::subscribe(const char* topic, uint8_t qos)
{
    _mutex.lock();
     if(!_connected){
        _mutex.unlock();
        return false;
     }
   _mutex.unlock();

  return  mqttClient.subscribe(topic, qos)!=0;;
}

bool  NetworkEngine::subscribe(String& topic, uint8_t qos)
{
    _mutex.lock();
     if(!_connected){
        _mutex.unlock();
        return false;
     }
   _mutex.unlock();
   
  return  mqttClient.subscribe(topic.c_str(), qos)!=0;;
}
bool  NetworkEngine::subscribe(const String&& topic, uint8_t qos)
{
    _mutex.lock();
     if(!_connected){
        _mutex.unlock();
        return false;
     }
   _mutex.unlock();
   
  return  mqttClient.subscribe(topic.c_str(), qos)!=0;;
}

void NetworkEngine::onMqttConnect(bool sessionPresent)
{
     printTrace("OK: Connected to MQTT.");
     // _mutex.lock();
      _connected=true;
     // _mutex.unlock();
      /*Serial.print("Session present: ");
      debug(sessionPresent);
      uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
      Serial.print("Subscribing at QoS 2, packetId: ");
      debug(packetIdSub);
      mqttClient.publish("test/lol", 0, true, "test 1");
      debug("Publishing at QoS 0");
      uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
      Serial.print("Publishing at QoS 1, packetId: ");
      debug(packetIdPub1);
      uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
      Serial.print("Publishing at QoS 2, packetId: ");
      debug(packetIdPub2);*/
}

void NetworkEngine::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
      printTrace("Disconnected from MQTT.");
        _mutex.lock();
      _connected=false;
       _mutex.unlock();
      if (WiFi.isConnected()) {
        setMqttReconnectTimer(true);
      }
      
    }
    
void NetworkEngine::onMqttSubscribe(uint16_t packetId, uint8_t qos)
 {
    printTrace("Subscribe acknowledged.");
    printTrace("packetId: "+String(packetId,DEC));
    printTrace("qos: "+String(qos,DEC));
}

void NetworkEngine::onMqttUnsubscribe(uint16_t packetId)
{
    printTrace("Unsubscribe acknowledged.");
    printTrace("packetId: "+String(packetId));
}
    /*
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
      debug("Publish received.");
      Serial.print("  topic: ");
      debug(topic);
      Serial.print("  qos: ");
      debug(properties.qos);
      Serial.print("  dup: ");
      debug(properties.dup);
      Serial.print("  retain: ");
      debug(properties.retain);
      Serial.print("  len: ");
      debug(len);
      Serial.print("  index: ");
      debug(index);
      Serial.print("  total: ");
      debug(total);
    }*/
void NetworkEngine::onMqttMessage(String&& topic,String&& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total) 
{
    printTrace("Publish received.");
    printTrace("  topic: ");
    printTrace(topic);
    printTrace("  payload: ");
    printTrace(payload);
    printTrace("  qos: "+String(properties.qos,DEC));
    printTrace("  dup: "+String(properties.dup,DEC));
    printTrace("  retain: "+String(properties.retain,DEC));
    printTrace("  len: "+String(payload.length(),DEC));
    printTrace("  index: "+String(index,DEC));
    printTrace("  total: "+String(total,DEC));
}
void NetworkEngine::onMqttPublish(uint16_t packetId) 
{
    printTrace("Publish acknowledged. \n");
    printTrace("  packetId: "+String(packetId,DEC));
}

void NetworkEngine::setMqttReconnectTimer(bool start)
{
      if(start){
          xTimerStart(_mqttReconnectTimer,0);
      }else{
          xTimerStop(_mqttReconnectTimer,0);
      }
}
void NetworkEngine::setWifiReconnectTimer(bool start)
{
      if(start){
          if(!WiFi.isConnected()){
            xTimerStart(_wifiReconnectTimer,0);
          }
          
      }else{
          xTimerStop(_wifiReconnectTimer,0);
      }
}

void NetworkEngine::startup(){
        WiFi.onEvent(std::bind(&NetworkEngine::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2));
      _wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)this, reinterpret_cast<TimerCallbackFunction_t>(&NetworkEngine::_thunkConnectToWifi));
      _mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)this, reinterpret_cast<TimerCallbackFunction_t>(&NetworkEngine::_thunkConnectToMqtt));
  
      mqttClient.onConnect(callback(this,&NetworkEngine::onMqttConnect));
      mqttClient.onDisconnect(callback(this,&NetworkEngine::onMqttDisconnect));
      mqttClient.onSubscribe(callback(this,&NetworkEngine::onMqttSubscribe));
      mqttClient.onUnsubscribe(callback(this,&NetworkEngine::onMqttUnsubscribe));
      mqttClient.onMessage(callback(this,&NetworkEngine::onMqttMessage));
      mqttClient.onPublish(callback(this,&NetworkEngine::onMqttPublish));
      mqttClient.setServer(MQTT_HOST, MQTT_PORT);

      _connectToWifi();

     #if !defined(NDEBUG)
     _threadMail.start(callback(this,&NetworkEngine::run_mail_box));
     #endif
}

void NetworkEngine::run_mail_box(){
  while(true){
    osEvent evt = _mail_box.get();
    if (evt.status == osEventMail) {
        email_t *mail = (email_t *)evt.value.p;
        for(auto& v: _delegateCallbacks){
            v.call(String("[NET]"),mail->message);
        }
        _mail_box.free(mail); 
    }
  }
  vTaskDelete(NULL);
}

void NetworkEngine::printTrace(const char* e)
{
  #if !defined(NDEBUG)
  printTrace(String(e));
  #endif
}

void NetworkEngine::printTrace(const String& e)
{
 #if !defined(NDEBUG)
  std_mutex.lock();
  email_t *mail = _mail_box.alloc();
  mail->message = String(e);
  _mail_box.put(mail) ;
  std_mutex.unlock();
#endif
}

void NetworkEngine::attach(Callback<void(const String&,const String&)> func)
{
    _delegateCallbacks.push_back(func);
}
  
