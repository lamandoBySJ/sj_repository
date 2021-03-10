#include <heltec.h>
//#include <ArduinoJson.h>
#include <rtos/rtos.h>
#include <app/TimeMachine/TimeMachine.h>
#include <app/ColorSensor/ColorSensor.h>
#include <ColorSensorBase.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Test.h>
#include <chrono>
#include <DelegateClass.h>

#include <esp_event_legacy.h>
#include <WiFiType.h>
#include <WiFi.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}

#include <app/AsyncMqttClient/AsyncMqttClient.h>

#define WIFI_SSID "360"
#define WIFI_PASSWORD "Aa000000"

#define MQTT_HOST IPAddress(192, 168, 1, 133)
#define MQTT_PORT 1883



using namespace mstd;
using namespace rtos;

#define BAND    433E6 
#if CONFIG_AUTOSTART_ARDUINO
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#endif

Test test;

using std::chrono::system_clock;

rtos::Mutex std_mutex;
DS1307 ds1307(Wire,21,22);
TimeMachine<DS1307> timeMachine(ds1307,std_mutex,13);  
//TimeMachine<RTCBase> timeMachine(&RTC,std_mutex);

rtos::Mutex mutex;
BH1749NUC bh1749nuc(Wire1,4,15);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,mutex,2);
//ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);

Thread thread1("Thd1",1024*2,1);

ExceptionCatcher e;
String ExceptionCatcher::exceptionType="";


class NetworkEngine
{
public:
    NetworkEngine()=default;
    NetworkEngine(const NetworkEngine& other)=default;
    NetworkEngine(NetworkEngine&& other)=default;
    ~NetworkEngine()=default;

    NetworkEngine& operator = (const NetworkEngine& that)=default;
    NetworkEngine& operator = (NetworkEngine&& that)=default;

    void onMqttConnect(bool sessionPresent) {
      Serial.println("Connected to MQTT.");
      Serial.print("Session present: ");
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
      Serial.println(packetIdPub2);
    }

    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
      Serial.println("Disconnected from MQTT.");

      if (WiFi.isConnected()) {
        xTimerStart(_mqttReconnectTimer, 0);
      }
    }

    void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
      Serial.println("Subscribe acknowledged.");
      Serial.print("  packetId: ");
      Serial.println(packetId);
      Serial.print("  qos: ");
      Serial.println(qos);
    }

    void onMqttUnsubscribe(uint16_t packetId) {
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
    void onMqttMessage(String&& topic,String&& payload, AsyncMqttClientMessageProperties properties, size_t index, size_t total) {
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
    void onMqttPublish(uint16_t packetId) {
      Serial.println("Publish acknowledged.");
      Serial.print("  packetId: ");
      Serial.println(packetId);
    }

    void setMqttReconnectTimer(bool start){
      if(start){
          xTimerStart(_mqttReconnectTimer,0);
      }else{
          xTimerStop(_mqttReconnectTimer,0);
      }
      
    }
    void setWifiReconnectTimer(bool start){
      if(start){
          xTimerStart(_wifiReconnectTimer,0);
      }else{
          xTimerStop(_wifiReconnectTimer,0);
      }
    }

    void startup(){
      _mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)this, reinterpret_cast<TimerCallbackFunction_t>(&NetworkEngine::thunkConnectToMqtt));
      _wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)this, reinterpret_cast<TimerCallbackFunction_t>(&NetworkEngine::thunkConnectToWifi));
      mqttClient.onConnect(callback(this,&NetworkEngine::onMqttConnect));
      mqttClient.onDisconnect(callback(this,&NetworkEngine::onMqttDisconnect));
      mqttClient.onSubscribe(callback(this,&NetworkEngine::onMqttSubscribe));
      mqttClient.onUnsubscribe(callback(this,&NetworkEngine::onMqttUnsubscribe));
      mqttClient.onMessage(callback(this,&NetworkEngine::onMqttMessage));
      mqttClient.onPublish(callback(this,&NetworkEngine::onMqttPublish));
      mqttClient.setServer(MQTT_HOST, MQTT_PORT);
      connectToWifi();
    }
    static void thunkConnectToWifi(void* pvTimerID) {
      static_cast<NetworkEngine*>(pvTimerID)->connectToWifi();
    }
     void connectToWifi() {
      Serial.println("Connecting to Wi-Fi...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    static void thunkConnectToMqtt(void* pvTimerID) {
      static_cast<NetworkEngine*>(pvTimerID)->connectToMqtt();
    }
    void connectToMqtt() {
      Serial.println("Connecting to MQTT...");
      mqttClient.connect();
    }
private:
    TimerHandle_t _mqttReconnectTimer;
    TimerHandle_t _wifiReconnectTimer;
    AsyncMqttClient mqttClient;
};

NetworkEngine networkEngine;

void WiFiEvent(system_event_id_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        networkEngine.connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection"); 
        // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        networkEngine.setMqttReconnectTimer(false);
        networkEngine.setWifiReconnectTimer(true);
        break;
    default:break;
    }
}



void setup() {
  
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true , false , true , true, BAND);
  //LoRa.dumpRegisters(Serial);
  timeMachine.attach(delegate(&e,&ExceptionCatcher::PrintTrace));
  timeMachine.startup();
  

  //timeMachine.setEpoch(1614764209+8*60*60);
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  colorSensor.attach(delegate(&e,&ExceptionCatcher::PrintTrace));
  colorSensor.startup();


  WiFi.onEvent(WiFiEvent);
  networkEngine.startup();
 //Callback<void(String,String,int)> call(&a,&A::Fun6);
 //call.call(String("ABC"),String("ABC"),1);

  //Callback<void(String&&,String&&,int&&)>  call();
  //MyTest t;
  //  A a;
 // t.Fun(Callback<void(String&&,String&&,int&&)>(&a,&A::Fun6));

}


std::array<uint16_t,4> dataRGB;
void loop() {
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
  // put your main code here, to run repeatedly:
   debug("__cplusplus:%ld\n",__cplusplus);
  /*
  colorSensor.measurementModeActive();
  colorSensor.getRGB(dataRGB);
  colorSensor.measurementModeInactive();
  debug("%d,%d,%d,%d\n",dataRGB[0],dataRGB[1],dataRGB[2],dataRGB[3]);
  */
  vTaskDelete(NULL);
}

  


