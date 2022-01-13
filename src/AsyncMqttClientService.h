#ifndef ASYNC_MQTT_CLIENT_SERVICE_H
#define ASYNC_MQTT_CLIENT_SERVICE_H

#include <arduino.h>
#include <app/AsyncMqttClient/AsyncMqttClient.h>
#include <WiFiType.h>
#include <WiFi.h>
#include <map>

#include <atomic>
#include <set>
#include <new>
#include <vector>
#include <mutex>
#include <thread>
#include <esp_event_legacy.h>
#include "platform_debug.h"
#include "platformio_api.h"

//#define TRACE_PRINTER_NDEBUG
enum class MqttEventType : int8_t {
  ON_MQTT_CONNECT = 0,
  ON_MQTT_DISCONNECT = 1,
  ON_MQTT_SUBSCRIBE = 2,
  ON_MQTT_UNSUBSCRIBE = 3,
  ON_MQTT_MESSAGE = 4,
  ON_MQTT_PUBLISH = 5,
  MQTT_CONNECT,
  MQTT_DISCONNECT,
  MQTT_PUBLISH,
  MQTT_SUBSCRIBE,
  MQTT_UNSUBSCRIBE,
  MQTT_CONNECT_ERROR,
  MQTT_DISCONNECT_ERROR,
  MQTT_PUBLISH_ERROR,
  MQTT_SUBSCRIBE_ERROR,
  MQTT_UNSUBSCRIBE_ERROR
};

struct mqtt_buffer_t :public os_memory_t //:public os_allocator_t
{

};



class MqttEventHandler
{
public:
    virtual ~MqttEventHandler()=default;
    virtual void onMqttConnect(bool sessionPresent)=0;
    virtual void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)=0;
    virtual void onMqttSubscribe(uint16_t packetId, uint8_t qos)=0;
    virtual void onMqttUnsubscribe(uint16_t packetId)=0;
    virtual void onMqttMessage(const char* topic,const char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)=0;
    virtual void onMqttPublish(uint16_t packetId)=0; 
};
class MqttEventListener : public MqttEventHandler
{
public:
    virtual ~MqttEventListener()=default;
    virtual const char* listener()=0;
    virtual void onMqttSubscribeError(const char* topic)=0;
    virtual void onMqttUnsubscribeError(const char* topic)=0;
    virtual void onMqttPublishError(const char* topic, uint8_t qos, bool retain, const char* payload, size_t length, bool dup, uint16_t message_id)=0;
    virtual void onMqttDisconnectError(AsyncMqttClientDisconnectReason reason)=0;
    virtual void onMqttConnectionLost()=0;
};

struct mqtt_event_t :public os_memory_t
{
    mqtt_event_t()=default;
    char topic[128];
    const char* payload;
    MqttEventType type;
    MqttEventListener* listener;
    bool sessionPresent;
    AsyncMqttClientDisconnectReason reason;
    uint16_t packetId;
    
    AsyncMqttClientMessageProperties properties;
    size_t len;
    size_t index;
    size_t total;
    
};

class AsyncMqttClientService : public MqttEventHandler 
{
public:

    AsyncMqttClientService()try:
    _threadTx(0,osPriorityNormal,1024*4,nullptr,"_thdTx"),
    _threadRx(0,osPriorityNormal,1024*4,nullptr,"_thdRx")
    {   

    }catch(const std::bad_alloc &e) {
        TracePrinter::printTrace(e.what());
        PlatformDebug::pause();
    }catch(const std::exception &e) {
        TracePrinter::printTrace(e.what());
        PlatformDebug::pause();
    }

     ~AsyncMqttClientService()=default;
  
    AsyncMqttClientService(const AsyncMqttClientService& other)=delete;
    AsyncMqttClientService(AsyncMqttClientService&& other)=delete;
    AsyncMqttClientService& operator = (const AsyncMqttClientService& that)=delete;
    AsyncMqttClientService& operator = (AsyncMqttClientService&& that)=delete;
   
    void onMqttConnect (bool sessionPresent)override
    {   
        _connected = true;
        #ifdef TRACE_PRINTER_NDEBUG
        TracePrinter::printTrace("[OK]: onMqttConnect.");
        #endif

        mqtt_event_t *mail = _mail_box_rx.alloc();
        if(mail!=NULL){
            mail->type=MqttEventType::ON_MQTT_CONNECT;
            mail->sessionPresent = sessionPresent;
            _mail_box_rx.put(mail) ;
        }
        
    }
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) override
    {
            _connected = false;
        #ifdef TRACE_PRINTER_NDEBUG
        TracePrinter::printTrace("[OK]: onMqttDisconnect."+String((int)reason,DEC));
        #endif
        Serial.printf("AsyncMqttClientDisconnectReason:%d\n",(int)reason);
        mqtt_event_t *mail = _mail_box_rx.alloc();
        if(mail!=NULL){
            mail->type=MqttEventType::ON_MQTT_DISCONNECT;
            mail->reason = reason;
            _mail_box_rx.put(mail) ;
        }
    }
        
    void onMqttSubscribe(uint16_t packetId, uint8_t qos)override
    {
        #ifdef TRACE_PRINTER_NDEBUG
        TracePrinter::printTrace("[OK]: onMqttSubscribe.");
        TracePrinter::printTrace("packetid:"+String(packetId)+String(": qos:") +String(qos));
        #endif
        mqtt_event_t *mail = _mail_box_rx.alloc();
        if(mail!=NULL){
            mail->type=MqttEventType::ON_MQTT_SUBSCRIBE;
            mail->packetId=packetId;
            mail->properties.qos=qos;
            _mail_box_rx.put(mail) ;
        }
    }

    void onMqttUnsubscribe(uint16_t packetId)override
    {
        #ifdef TRACE_PRINTER_NDEBUG
        TracePrinter::printTrace("[OK]: onMqttUnsubscribe.");
        TracePrinter::printTrace("packetId: "+String(packetId));
        #endif
        mqtt_event_t *mail = _mail_box_rx.alloc();
        if(mail!=NULL){
            mail->type=MqttEventType::ON_MQTT_UNSUBSCRIBE;
            mail->packetId=packetId;
            _mail_box_rx.put(mail) ;
        }
      
    }

    void onMqttMessage(const char* topic,const char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) override
    {   
        #ifdef TRACE_PRINTER_NDEBUG
        TracePrinter::printTrace("[OK]: onMqttMessage.");
        #endif
        mqtt_event_t *mail = _mail_box_rx.alloc();
        if(mail){
            mail->type=MqttEventType::ON_MQTT_MESSAGE;
            snprintf(mail->topic,sizeof(mail->topic),"%s",topic);
            mail->payload= mail->copy("%.*s",len,payload);
            mail->properties.qos = properties.qos;
            mail->properties.dup = properties.dup;
            mail->properties.retain = properties.retain;
            mail->index = index;
            mail->total = total;
            _mail_box_rx.put(mail) ;
        }
    }
    
    void onMqttPublish(uint16_t packetId) override
    {
        #ifdef TRACE_PRINTER_NDEBUG
        TracePrinter::printTrace("[OK]: onMqttPublish.");
        TracePrinter::printTrace("  packetId: "+String(packetId,DEC));
        #endif
        mqtt_event_t *mail = _mail_box_rx.alloc();
        if(mail!=NULL){
            mail->type=MqttEventType::ON_MQTT_PUBLISH;
            mail->packetId=packetId;
            _mail_box_rx.put(mail) ;
        }
    }
    void WiFiEvent(system_event_id_t event, system_event_info_t info);

    void eventHandlerRegister(MqttEventListener* listener){
        _MqttEventListener.insert(listener);
    }
    void eventHandlerUnregister(MqttEventListener* listener){
        _MqttEventListener.erase(listener);
    }
    void eventTxLooper();
    void eventRxLooper();

    
    void init(const char* host,uint16_t port,const char* id="IPSBox");

    void addMqttEventListener(MqttEventListener* mqttEventListener){
        if(mqttEventListener){
            _MqttEventListener.insert(mqttEventListener);
        }
    }  
    static void shutdown(){
        getInstance()._shutdown();
    }
    void _shutdown(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        if(_threadTx.get_state()==Thread::Running){
            _threadTx.terminate();
           _threadRx.terminate(); 
        }
    }
    static void startup(const char* host,uint16_t port,const char* id,MqttEventListener* mqttEventlistner=nullptr) 
    {   
        if(mqttEventlistner!=nullptr){
            getInstance().addMqttEventListener(mqttEventlistner);
        }
        getInstance().init(host,port,id);
    }
    static AsyncMqttClientService& getInstance(){
        static AsyncMqttClientService* clientService=new AsyncMqttClientService();
        return *clientService;
    }
    static void connect(){
        getInstance()._connect();
        #if defined TRACE_PRINTER_NDEBUG
        TracePrinter::printTrace("MQTT Client Connecting...");
        #endif
    }
    void _connect(){
        std::lock_guard<rtos::Mutex> lck(_mtx);
        mqtt_event_t *mail = _mail_box_tx.alloc();
        if(mail){
            mail->type=MqttEventType::MQTT_CONNECT;
            _mail_box_tx.put(mail);
        }else{
            TracePrinter::printf("%s\n",__PRETTY_FUNCTION__);
        }
    }
    static void disconnect(){
       getInstance().postMailBoxTx(MqttEventType::MQTT_DISCONNECT);
       #if defined TRACE_PRINTER_NDEBUG
       TracePrinter::printTrace("MQTT Client disconnect!");
       #endif
    }


    template <typename... T>
    static inline bool publish(const char* topic,bool retain,uint8_t qos,const char* payload,T&&... args)
    {
        return getInstance()._publish(topic,retain,qos,payload,std::forward<T>(args)...);
    }
    template <typename... T>
    static inline bool publish(const String& topic,bool retain,uint8_t qos,const char* payload,T&&... args)
    {
        return getInstance()._publish(topic.c_str(),retain,qos,payload,std::forward<T>(args)...);
    }
   


    static bool publish(const char* topic=nullptr,uint8_t qos=0,bool retain=false,const char* payload=nullptr,size_t length = 0, bool dup = false,uint16_t message_id=0){
        return getInstance().postMailBoxTx(MqttEventType::MQTT_PUBLISH,topic,qos,retain,payload,length,dup,message_id);
    }
    static bool publish(const String& topic,const String& payload, uint8_t qos=0, bool retain=false,bool dup=false, uint16_t message_id=0)
    {
        return getInstance().postMailBoxTx(MqttEventType::MQTT_PUBLISH,topic.c_str(),qos,retain,payload.c_str(),payload.length(),dup,message_id);
    }   





    static bool subscribe(const char* topic,uint8_t qos=0){
        return getInstance().postMailBoxTx(MqttEventType::MQTT_SUBSCRIBE,topic,qos,false,nullptr,0,false,0);
    }
    static bool unsubscribe(char* topic){
        return getInstance().postMailBoxTx(MqttEventType::MQTT_UNSUBSCRIBE,topic,0,false,nullptr,0,false,0);
    }

    static bool  subscribe(const String& topic, uint8_t qos=0)
    {
        return getInstance().postMailBoxTx(MqttEventType::MQTT_SUBSCRIBE,topic.c_str(),qos,false,nullptr,qos,false,false);
    }
    static bool unsubscribe(const String& topic)
    { 
        return getInstance().postMailBoxTx(MqttEventType::MQTT_UNSUBSCRIBE,topic.c_str(),0,false,nullptr,0,false,0); 
    }
    static void setClientID(const char* clientId){
        getInstance()._clientId=clientId;
    }
    static bool connected(){
       return getInstance()._connected.load();
    }
    bool postMailBoxTx(MqttEventType type,const char* topic=nullptr, uint8_t qos=0, bool retain=false, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0);

protected:
    template <typename... T>
    bool _publish(const char* topic,bool retain,uint8_t qos,const char* payload,T&&... args){
       return getInstance().postMailBoxTx(MqttEventType::MQTT_PUBLISH,topic,qos,retain,_mqtt_buffer.copy(payload,std::forward<T>(args)...),_mqtt_buffer.length(),false,0);
    }
private:
    
    //static IPAddress MQTT_HOST ;
    rtos::Mail<mqtt_event_t, 8> _mail_box_tx;
    rtos::Mail<mqtt_event_t, 8> _mail_box_rx;

    std::set<MqttEventListener*> _MqttEventListener;
    rtos::Thread _threadTx;
    rtos::Thread _threadRx;
    rtos::Mutex _mtx;

    AsyncMqttClient _client;
    const char* _host;
    uint16_t _port;  
    const char* _clientId;
    mqtt_buffer_t _mqtt_buffer;
    std::atomic<bool> _connected;
};

#endif