#ifndef MQTT_NETWORK_H
#define MQTT_NETWORK_H

#include <arduino.h>
#include <app/AsyncMqttClient/AsyncMqttClient.h>
#include <WiFiType.h>
#include <WiFi.h>
#include <rtos/rtos.h>
#include "platform_debug.h"
#include <map>
#include <set>
#include <new>
#include <vector>
#include <mutex>
#include <thread>
#include "platform/mbed.h"
#include "rtos/Thread.h"

namespace mqtt
{
struct [[gnu::may_alias]]  test_t{ 
    uint32_t counter=0; 
} ;
struct [[gnu::may_alias]]  test1_t{ 
    uint32_t counter=0; 
} ;
struct [[gnu::may_alias]]  test2_t{ 
     uint32_t counter=0; 
   // AsyncMqttClientMessageProperties properties;
   // size_t index;
   // size_t total;
   // char topic[128];
    //String payload;

} ;
struct [[gnu::may_alias]]  mail_subscribe_t {
    bool sessionPresent;  
    uint32_t counter=0; 
} ;
struct [[gnu::may_alias]] mail_wifi_event_t{
    system_event_id_t id;
     
};
struct [[gnu::may_alias]] mail_mqtt_t {
    size_t index;
    size_t total;
    AsyncMqttClientMessageProperties properties;
    String topic;
    String payload;
};

}

class MQTTNetwork
{
public:
    MQTTNetwork()try:
    _threadWiFiEvent(0,osPriorityNormal,1024*2,nullptr,"thdWiFiEvent"),
    _threadOnMessage(0,osPriorityNormal,1024*2,nullptr,"thdOnMessage"),
    _threadSubscribe(0,osPriorityNormal,1024*2,nullptr,"thdSubscribe"),
    _autoConnect(true)
    {   
        _wifiCB=std::bind(&MQTTNetwork::WiFiEvent,this,std::placeholders::_1,std::placeholders::_2);

    }catch(const std::bad_alloc &e) {
        TracePrinter::printTrace(e.what());
        PlatformDebug::pause();
    }catch(const std::exception &e) {
        TracePrinter::printTrace(e.what());
        PlatformDebug::pause();
    }

     ~MQTTNetwork()=default;
    MQTTNetwork(const MQTTNetwork& other)=default;
    MQTTNetwork(MQTTNetwork&& other)=default;
   

    MQTTNetwork& operator = (const MQTTNetwork& that)=default;
    MQTTNetwork& operator = (MQTTNetwork&& that)=default;

    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttSubscribe(uint16_t packetId, uint8_t qos);
    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(char* topic,char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) ;
    //void onMqttMessage(const String& topic,const String& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);

    void startup();  //throw (os::thread_error)
  
    void switchWiFiMode(wifi_mode_t mode){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        WiFi.mode(mode);
    }
    
    bool connected();
    //void _connectToMqtt();
    //void _connectToWifi();
    // void setMqttReconnectTimer(bool start);
    // void setWifiReconnectTimer(bool start);
    // static void _thunkConnectToWifi(void* pvTimerID);
    // static void _thunkConnectToMqtt(void* pvTimerID);
    std::function<void(system_event_id_t event, system_event_info_t info)>  _wifiCB;
    void disconnect(bool autoConnect=false);
    bool publish(const String& topic,const String& payload, uint8_t qos=0, bool retain=false,bool dup=true, uint16_t message_id=0);
    bool subscribe(const String& topic, uint8_t qos=0);
    bool unsubscribe(const String& topic);
   
    void WiFiEvent(system_event_id_t event, system_event_info_t info);
    void addOnMessageCallback(Callback<void(const String&,const String&)> func);
    void addSubscribeTopic(const String& topic,int qos=0);
    void addSubscribeTopics(std::vector<String>& vec);
    void addOnMqttConnectCallback(Callback<void(bool)> func);
    void addOnMqttDisonnectCallback(Callback<void(AsyncMqttClientDisconnectReason)> func);
    
   
    void runWiFiEventService();
    void run_mail_box_on_message_arrived();
    void run_mail_box_topics_subscribed();
    void init();

protected:
    
private:
    rtos::Mail<mqtt::test_t, 3> _mail_box_test;
  rtos::Mail<mqtt::test1_t, 3> _mail_box_test1;
      rtos::Mail<mqtt::test2_t, 3> _mail_box_test2;

    rtos::Mail<mqtt::mail_wifi_event_t, 4> _mailBoxWiFiEvent;
    rtos::Mail<mqtt::mail_mqtt_t, 8> _mail_box_on_message;
    rtos::Mail<mqtt::mail_subscribe_t, 2> _mail_box_on_subscribe;

    
    //static IPAddress MQTT_HOST ;
    rtos::Thread _threadWiFiEvent;
    rtos::Thread _threadOnMessage;
    rtos::Thread _threadSubscribe;

    

    rtos::Mutex _mtx;

    std::vector<mbed::Callback<void(const String&,const String&)>>  _onMessageCallbacks;
    std::vector<mbed::Callback<void(bool)>>  _onMqttConnectCallbacks;
    std::vector<mbed::Callback<void(AsyncMqttClientDisconnectReason)>>  _onMqttDisconnectCallbacks;
    std::set<String>  _topics;

    AsyncMqttClient _client;
    bool _autoConnect;
};

#endif