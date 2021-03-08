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
#include <DelegateClass.hpp>

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
Thread thread2("Thd2",1024*2,2);

ExceptionCatcher e;
String ExceptionCatcher::exceptionType="";

MFRC522_UART uartDevice =MFRC522_UART(21,Serial2);
MFRC522 rfid = MFRC522(&uartDevice); 

void setup() {
  
  pinMode(18,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(19,OUTPUT);
  // put your setup code here, to run once:
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(true , false , true , true, BAND);
  //LoRa.dumpRegisters(Serial);
  /*timeMachine.attach(delegate(&e,&ExceptionCatcher::PrintTrace));
  timeMachine.startup();
  

  //timeMachine.setEpoch(1614764209+8*60*60);
  ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  colorSensor.attach(delegate(&e,&ExceptionCatcher::PrintTrace));
  colorSensor.startup();
*/
  rfid.PCD_Init(); // Init MFRC522 
}

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

std::array<uint16_t,4> dataRGB;
void loop() {
  
  // put your main code here, to run repeatedly:
 // debug("%s,__cplusplus:%ld\n",data,__cplusplus);
 /* ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
  colorSensor.measurementModeActive();
  colorSensor.getRGB(dataRGB);
  colorSensor.measurementModeInactive();
  debug("%d,%d,%d,%d\n",dataRGB[0],dataRGB[1],dataRGB[2],dataRGB[3]);
  */
 Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
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
  //vTaskDelete(NULL);
}

  


