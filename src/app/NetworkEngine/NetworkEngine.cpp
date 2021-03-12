#include "../NetworkEngine/NetworkEngine.h"


void NetworkEngine::WiFiEvent(system_event_id_t event, system_event_info_t info) {
 
    Serial.printf("[WiFi-event] event: %d\n", event,info);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        setMqttReconnectTimer(true);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection"); 
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
IPAddress NetworkEngine::MQTT_HOST(192, 168, 1, 133);
uint16_t NetworkEngine::MQTT_PORT =1883;
const char* NetworkEngine::WIFI_SSID="IPS";
const char* NetworkEngine::WIFI_PASSWORD="Aa000000";

void NetworkEngine::_thunkConnectToWifi(void* pvTimerID)
{
    Serial.println("_thunkConnectToWiFi...");
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
     static_cast<NetworkEngine*>(pvTimerID)->_connectToWifi();
}
void NetworkEngine::_thunkConnectToMqtt(void* pvTimerID) 
{
      Serial.println("_thunkConnectToMQTT...");
      static_cast<NetworkEngine*>(pvTimerID)-> _connectToMqtt();
}
void NetworkEngine::_connectToMqtt()
{
        Serial.println("Connecting to MQTT...");
         _mutex.lock();
        mqttClient.connect();
        _mutex.unlock();
}
void NetworkEngine::_connectToWifi() 
{
        Serial.println("Connecting to Wi-Fi...");
        _mutex.lock();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        _mutex.unlock();
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
      Serial.println("Connected to MQTT.");
      _mutex.lock();
      _connected=true;
      _mutex.unlock();
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
      Serial.println("Disconnected from MQTT.");
        _mutex.lock();
      _connected=false;
     
      if (WiFi.isConnected()) {
        //xTimerStart(_mqttReconnectTimer, 0);
        setMqttReconnectTimer(true);
      }
       _mutex.unlock();
    }
    
void NetworkEngine::onMqttSubscribe(uint16_t packetId, uint8_t qos)
 {
      Serial.println("Subscribe acknowledged.");
      Serial.print("  packetId: ");
      Serial.println(packetId);
      Serial.print("  qos: ");
      Serial.println(qos);
}

void NetworkEngine::onMqttUnsubscribe(uint16_t packetId)
{
      Serial.println("Unsubscribe acknowledged.");
      Serial.print("  packetId: ");
      Serial.println(packetId);
}
    /*
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
      Serial.println("Publish received.");
      Serial.print("  topic: ");
      Serial.println(topic);
      Serial.print("  qos: ");
      Serial.println(properties.qos);
      Serial.print("  dup: ");
      Serial.println(properties.dup);
      Serial.print("  retain: ");
      Serial.println(properties.retain);
      Serial.print("  len: ");
      Serial.println(len);
      Serial.print("  index: ");
      Serial.println(index);
      Serial.print("  total: ");
      Serial.println(total);
    }*/
void NetworkEngine::onMqttMessage(String&& topic,String&& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total) 
{
      Serial.println("Publish received.");
      Serial.print("  topic: ");
      Serial.println(topic);
      Serial.println(payload);
      Serial.print("  qos: ");
      Serial.println(properties.qos);
      Serial.print("  dup: ");
      Serial.println(properties.dup);
      Serial.print("  retain: ");
      Serial.println(properties.retain);
      Serial.print("  len: ");
      Serial.println(payload.length());
      Serial.print("  index: ");
      Serial.println(index);
      Serial.print("  total: ");
      Serial.println(total);
}
void NetworkEngine::onMqttPublish(uint16_t packetId) 
{
      Serial.println("Publish acknowledged.");
      Serial.print("  packetId: ");
      Serial.println(packetId);
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
     // _coreThread.start(callback(this,run));
}

void NetworkEngine::run(){

}
  