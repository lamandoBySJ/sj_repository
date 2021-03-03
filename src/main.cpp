#include <heltec.h>
//#include <ArduinoJson.h>
#include <rtos/rtos.h>
#include <app/TimeMachine/TimeMachine.h>
#include <app/ColorSensor/ColorSensor.h>
#include <ColorSensorBase.h>
#include <SPI.h>
#include <MFRC522.h>

#include <chrono>

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

using std::chrono::system_clock;

rtos::Mutex std_mutex;
DS1307 RTC(Wire,32,33);
TimeMachine<DS1307> timeMachine(RTC,std_mutex);
TimeMachine<RTCBase> timeMachine2(&RTC,std_mutex);

rtos::Mutex mutex;
BH1749NUC bh1749nuc(Wire1,21,22,100000);
ColorSensor<BH1749NUC> colorSensor(bh1749nuc,mutex);
ColorSensor<ColorSensorBase> colorSensor2(&bh1749nuc,mutex);

Thread thread3("Thd3",1024*2,3);
Thread thread4("Thd4",1024*2,4);
Thread thread5("Thd5",1024*2,5);
Thread thread6("Thd6",1024*2,6);
Thread thread7("Thd7",1024*2,7);

Thread thread1("Thd1",1024*2,1);
Thread thread2("Thd2",1024*2,2);

char data[]="hello sj~";

void TaskDebug( void *pvParameters );

void TaskTest0()
{
 String datetime="";
  for(;;){
     // stdmutex.lock();
      String&& datetime = timeMachine.getDateTime();
      debug("Test0: __cplusplus:%s , %s\n", String(__cplusplus,DEC).c_str(),datetime.c_str() );
      ThisThread::sleep_for(Kernel::Clock::duration_u32(10000));
      //stdmutex.unlock();
  }
}

void TaskTest(int *pvParameters  )
{
 
  for(;;){
     // stdmutex.lock();
      String&& datetime = timeMachine.getDateTime();
      debug("Test1: __cplusplus:%s , %s\n", String(__cplusplus,DEC).c_str(),datetime.c_str() );
      ThisThread::sleep_for(Kernel::Clock::duration_u32(10000));
      //stdmutex.unlock();
  }
}
int (*_call)();
template <typename R>
class Work : private detail::CallbackBase
{
public:
  auto call() -> decltype(_call) 
  {
    //MBED_ASSERT(bool(*this));
    return _call;
  }
};

class Test
{
public:
    Test(){};
    ~Test(){};
    void run(){
      
      for(;;){
        String&& datetime = timeMachine.getDateTime();
        debug("Callback: ESP.getFreeHeap():%d ,%s\n",ESP.getFreeHeap(),datetime.c_str() );
        ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
      }
    }
 
    void startup(){
      thread.start(callback(this,&Test::run));
    }
private:
    Thread thread;
};
int a =1993;
Test test;
//Serial.println(uxTaskPriorityGet(myTask));configMAX_PRIORITIES
constexpr uint8_t RST_PIN = 22;          // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN =  16;         // Configurable, see typical pin layout above

//SPIClass spiBus(HSPI);
//MFRC522_SPI spiDevice =MFRC522_SPI(SS_PIN, RST_PIN,&spiBus);
//MFRC522 rfid = MFRC522(&spiDevice); 

//SPIClass spiBus(HSPI);
MFRC522_UART uartDevice =MFRC522_UART(SS_PIN, RST_PIN);
MFRC522 rfid = MFRC522(&uartDevice); 

void setup() {
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true , false , true , true, BAND);
  //LoRa.dumpRegisters(Serial);

  /* timeMachine.startup(NULL);
  int timeout= 3;
  do
  {
    ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
    if(timeout-- == 0){
        //todo send message
        debug("RTC ERROR,please check out RTC DS1307,cplusplus:%ld\n",__cplusplus);
        break;
    }
  }while(timeMachine.getEpoch()==0);

  timeMachine.setEpoch(1614236396+8*60*60);
  String&& debugtime=RTC.getDateTime();
  debug("RTC:%d,%s\n",(int)RTC.getEpoch(),debugtime.c_str());

  ThisThread::sleep_for(Kernel::Clock::duration_u32(3000));
  test.startup();
 thread1.start(callback(TaskTest0));
  thread2.start(callback(TaskTest,&a));

  thread3.start(callback(TaskTest0));
  thread4.start(callback(TaskTest0));
  thread5.start(callback(TaskTest0));
  thread6.start(callback(TaskTest0));
  thread7.start(callback(TaskTest0));*/
  //spiBus.begin(26, 12, 13,15);

 
 
  
  for(;;){
    static system_clock::time_point today = system_clock::now();
    std::time_t tt = system_clock::to_time_t(today);
    debug( "today is:%s\n ",ctime(&tt));
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  }
}



//MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
 // debug("%s,__cplusplus:%ld\n",data,__cplusplus);
  //vTaskDelete(NULL);
 rfid.PCD_Init(); // Init MFRC522 
 // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent()){

     Serial.println(F("No found RFID Card..."));
     ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
     return;
  }
   

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial()){
    Serial.println(F("Not PICC_ReadCardSerial"));
    ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
    return;
  }
   

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    nuidPICC[0]=0;
    nuidPICC[1]=0;
    nuidPICC[2]=0;
    nuidPICC[3]=0;
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  
}

void TaskDebug( void *pvParameters )
{
  //int cnt=0;
  for(;;){
     // vTaskResume(handleTaskDebug);
      //Serial.println("hello");
    //debug_if(true,"debug_if:%d\n",data);
    //delay(10000);
   // ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
      //debug("task debug ...%d\n",++x);
      //ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
  }
  
}

 /*
  xTaskCreatePinnedToCore(
    TaskDebug
    ,  "TaskDebug"
    ,  8*1024  
    ,  NULL
    ,  1  
    ,  &handleTaskDebug
    ,  ARDUINO_RUNNING_CORE);
  attachInterrupt(0, []  {
    vTaskSuspend(handleTaskDebug);
  }, FALLING);   

  */
  


