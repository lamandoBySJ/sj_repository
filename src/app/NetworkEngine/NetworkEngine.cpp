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
 
    platform_debug::TracePrinter::printTrace("[WiFi-event] event: "+String(event,DEC));

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        platform_debug::TracePrinter::printTrace("WiFi connected");
        platform_debug::TracePrinter::printTrace("IP address: "+WiFi.localIP().toString());
        setMqttReconnectTimer(true);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        platform_debug::TracePrinter::printTrace("WiFi lost connection"); 
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

bool NetworkEngine::unsubscribe(const String& topic)
{ 
   return unsubscribe(topic.c_str());
}

bool NetworkEngine::unsubscribe(const char* topic)
{
  _mutex.lock();
  uint16_t id = mqttClient.unsubscribe(topic);
  _mutex.unlock();
  return id!=0;
}

void NetworkEngine::onMqttConnect(bool sessionPresent)
{
     platform_debug::TracePrinter::printTrace("OK: Connected to MQTT.");
     _connected=true;
     

     for(auto& v : _topics){
        platform_debug::TracePrinter::printTrace("Topic subscribe: "+v);
        subscribe(v, 0);
     }

    for(auto& v : _onMqttConnectCallbacks){
        v.call(sessionPresent);
     }
     // uint16_t packetIdSub = mqttClient.subscribe("test", 0);
     // _mutex.lock();
      
     // _mutex.unlock();
      /*Serial.print("Session present: ");
      Serial.println(sessionPresent);
      uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
      Serial.print("Subscribing at QoS 2, packetId: ");
      Serial.println(packetIdSub);
      mqttClient.publish("test/lol", 0, true, "test 1");
      Serial.println("Publishing at QoS 0");
      uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
      Serial.print("Publishing at QoS 1, packetId: ");
      Serial.println(packetIdPub1);
      uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
      Serial.print("Publishing at QoS 2, packetId: ");
      Serial.println(packetIdPub2);*/
}

void NetworkEngine::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
      _connected=false;
      platform_debug::TracePrinter::printTrace("Disconnected from MQTT.");
      
      for(auto& v : _onMqttDisconnectCallbacks){
        v.call(reason);
      }

      if (WiFi.isConnected()) {
        setMqttReconnectTimer(true);
      }
      
    }
    
void NetworkEngine::onMqttSubscribe(uint16_t packetId, uint8_t qos)
 {
    platform_debug::TracePrinter::printTrace("[OK]:Subscribe acknowledged.");
    platform_debug::TracePrinter::printTrace("packetId: "+String(packetId,DEC));
    platform_debug::TracePrinter::printTrace("qos: "+String(qos,DEC));
}

void NetworkEngine::onMqttUnsubscribe(uint16_t packetId)
{
    platform_debug::TracePrinter::printTrace("[OK]:Unsubscribe acknowledged.");
    platform_debug::TracePrinter::printTrace("packetId: "+String(packetId));
}
  
    void NetworkEngine::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
    {
      
      mail_t *mail = _mail_box.alloc();
      if(mail!=NULL){
        mail->topic = topic;
        mail->payload = String(payload,len);
        mail->properties.qos = properties.qos;
        mail->properties.dup = properties.dup;
        mail->properties.retain = properties.retain;
        mail->index = index;
        mail->total = total;
        _mail_box.put(mail) ;
      }
     
      /*
      Serial.println("Publish received.");
      Serial.print("  topic: ");
      Serial.print(topic);
      Serial.print("  payload: ");
       Serial.print(payload);
      Serial.print("  qos: ");
       Serial.print(properties.qos);
      Serial.print("  dup: ");
       Serial.print(properties.dup);
      Serial.print("  retain: ");
       Serial.print(properties.retain);
      Serial.print("  len: ");
       Serial.print(len);
      Serial.print("  index: ");
       Serial.print(index);
      Serial.print("  total: ");
       Serial.print(total);
      */
    }
  
void NetworkEngine::onMqttPublish(uint16_t packetId) 
{
    platform_debug::TracePrinter::printTrace("Publish acknowledged. \n");
    platform_debug::TracePrinter::printTrace("  packetId: "+String(packetId,DEC));
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

     _threadMail.start(callback(this,&NetworkEngine::run_mail_box));
}



void NetworkEngine::run_mail_box(){
 
  while(true){
   
    osEvent evt= _mail_box.get();
    if (evt.status == osEventMail) {
        mail_t *mail = (mail_t *)evt.value.p;
        for(auto& v:  _onMessageCallbacks){
              v.call(mail->topic,mail->payload);
              Serial.println(mail->topic);
              Serial.println(mail->payload);
        }
        
        _mail_box.free(mail); 
    }
  
  }
 // vTaskDelete(NULL);
}

void NetworkEngine::addOnMessageCallback(Callback<void(const String&,const String&)> func)
{
    _onMessageCallbacks.push_back(func);
}
void NetworkEngine::addTopic(const String& topic,int qos)
{
    _topics.push_back(topic);
}
void NetworkEngine::addOnMqttConnectCallback(Callback<void(bool)> func)
{
   _onMqttConnectCallbacks.push_back(func);
}
void NetworkEngine::addOnMqttDisonnectCallback(Callback<void(AsyncMqttClientDisconnectReason)> func)
{
   _onMqttDisconnectCallbacks.push_back(func);
}


  
