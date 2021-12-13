#include <heltec.h>
//#include <SPI.h>
//#include <MFRC522.h>
#include "esp_sleep.h"
#include "LoopTaskGuard.h"
#include "IPSBox.h"
#include "platform_debug.h"
#include "Logger.h"
#include "app/RTC/RTC.h"
#include "app/LED/LED.h"
#include "app/Alarm/Alarm.h"
#include <DS3231M.h>
#include "esp_sleep.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp32/ulp.h"
#include "drivers/InterruptIn.h"
#include "sx1278-mbed.h"
#include "driver/rtc_io.h"
#include "UlpDebug.h"
#include "registers.h"
#include "esp_sleep.h"
#include <mutex>
#include "Button.h"
#include "driver/adc.h"
#include "app/Battery/BatteryTemp.h"
#include "app/Battery/BatteryVoltage.h"
using namespace mbed;
using namespace std;
#define BAND    470E6 

//,public SX1278Interface
// PINOUT FOR MBED_NUCLEO_L053R8 BOARD
#define SX1278_MOSI  IO27
#define SX1278_MISO  IO19
#define SX1278_SCLK  IO5
#define SX1278_NSS   IO18
#define SX1278_RST   IO14
#define SX1278_DIO0  IO26
#define SX1278_DIO1  IO35
#define SX1278_DIO2  IO34
#define SX1278_DIO3  NC
#define SX1278_DIO4  NC
#define SX1278_DIO5  NC

// Defines
#define DEBUG_ON 1




bool deepsleep=false;
bool timeout_flag=false;
// Slow memory variable assignment
enum {
  SLOW_BLINK_STATE,     // Blink status
  SLOW_SPI_END,     
  SLOW_SPI_VAL_WR, 
  SLOW_SLEEP_MODE,       
  SLOW_PROG_ADDR        // Program start address
};
   
enum class Radio315: char{
  SYNC,
  DETECTED,     
};
rtos::Mutex mtx;
void ULP_BLINK(uint32_t us) {
    // Blink status initialization
      RTC_SLOW_MEM[SLOW_BLINK_STATE] =  0;
      RTC_SLOW_MEM[1] = 0;
      RTC_SLOW_MEM[3]=0;
    
     // ULP Program
    //I_MOVI(R3, SLOW_BLINK_STATE),           // R3 = SLOW_BLINK_STATE
    //I_LD(R0, R3, 0),                        // R0 = RTC_SLOW_MEM[R3(SLOW_BLINK_STATE)]
    //M_BL(1, 1),                             // IF R0 < 1 THAN GOTO M_LABEL(1)
  
    //I_WR_REG(RTC_GPIO_OUT_REG, pin_spi_nss_bit, pin_spi_nss_bit, 0),
    // R0 => 1 : run
    /*I_WR_REG(RTC_GPIO_OUT_REG, pin_blink_bit, pin_blink_bit, 1), // pin_blink_bit = 1
    I_MOVI(R0, 0),                          // R0 = 0
    I_ST(R0, R3, 0),                        // RTC_SLOW_MEM[R3(SLOW_BLINK_STATE)] = R0
    M_BX(2),                                // GOTO M_LABEL(2)
 
    // R0 < 1 : run
    M_LABEL(1),                             // M_LABEL(1)
    I_WR_REG(RTC_GPIO_OUT_REG, pin_blink_bit, pin_blink_bit, 0),// pin_blink_bit = 0
    I_MOVI(R0, 1),                          // R0 = 1
    I_ST(R0, R3, 0),                        // RTC_SLOW_MEM[R3(SLOW_BLINK_STATE)] = R0
    */



  // PIN to blink (specify by +14)
  const int pin_input_bit = RTCIO_GPIO4_CHANNEL + 14;
  const gpio_num_t pin_input = GPIO_NUM_4;

  const int pin_reset_bit = RTCIO_GPIO14_CHANNEL + 14;
  const gpio_num_t pin_reset = GPIO_NUM_14;

  const int pin_blink_bit = RTCIO_GPIO33_CHANNEL + 14;
  const gpio_num_t pin_blink = GPIO_NUM_33;

  const int pin_spi_sck_bit = RTCIO_GPIO25_CHANNEL + 14;
  const gpio_num_t pin_spi_sck = GPIO_NUM_25;
  const int pin_spi_mosi_bit = RTCIO_GPIO27_CHANNEL + 14;
  const gpio_num_t pin_spi_mosi = GPIO_NUM_27;
  const int pin_spi_miso_bit = RTCIO_GPIO36_CHANNEL + 14;
  const gpio_num_t pin_spi_miso = GPIO_NUM_36;
  const int pin_spi_nss_bit = RTCIO_GPIO32_CHANNEL + 14;
  const gpio_num_t pin_spi_nss = GPIO_NUM_32;

  // GPIO26 initialization (set to output and initial value is 0)
 
 const ulp_insn_t  ulp_prog[] = {
    M_LABEL(0),
  };
  // Run the program shifted backward by the number of variables
 
  // Set ULP activation interval
  ulp_set_wakeup_period(0, us);
  if(!deepsleep){
        // Slow memory initialization
      memset(RTC_SLOW_MEM, 0, 8192);
      size_t size = sizeof(ulp_prog) / sizeof(ulp_insn_t);
      ulp_process_macros_and_load(SLOW_PROG_ADDR, ulp_prog, &size);
  }
  ulp_run(SLOW_PROG_ADDR);
}
// Radio events function pointer
//static RadioEvents_t RadioEvents;

const char GatewayMsg[] = "Hello FROM LORA GATEWAY!";
const char ClientMsg[] = "Hello FROM LORA CLIENT!";
//python3 esptool.py --port COM20 --baud 115200 write_flash -fm dio -fs 16MB  0x410000 partitions.bin
//C:\Users\Administrator\.platformio\packages\framework-arduinoespressif32\tools\partitions
/*libraries download:
1.ESP32httpUpdate
2.ESPAsyncWebServer
*/
__attribute__((constructor))
void before() {
    Alarm::init();
}

IPSBox* box=new IPSBox();
SX1278 sx1278;
//#define OLEDSCREEN 
volatile bool flag=false;

volatile uint64_t interval=0;

#define BTN_STOP_ALARM    0
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * timer = NULL;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;
#define BTN_STOP_ALARM    0

unsigned int Rf_Cnt;
unsigned int Lo_Cnt;
unsigned int Hi_Cnt;

unsigned int Count_Lead;
unsigned int Count_Data_Hi;
unsigned int Count_Data_Lo;

unsigned int Recv_Bit_Cnt;
unsigned int Recv_Byte_Cnt;
unsigned int Recv_Data_Buf;
unsigned int Rf_Control_Data;
unsigned int Rf_Data[3];
bool complete=false;
void IRAM_ATTR onTimer(void* arg){}

static esp_timer_handle_t esp_timer=NULL;
static SemaphoreHandle_t timerSemaphore=NULL;

void transmit(uint8_t pin,int cnt,char* data);
//static char encodeAlert[5]={0x51,0x2A,0x11,0xFF,0xFF};
//static char encode[5]     ={0x51,0x2A,0x10,0xFF,0xFF};
static char FMD_ENCODE_TAG_SETUP[5] = {0x51,0x2A,0x00,0x51,0x2A};
static char FMD_ENCODE_TAG_ALERT[5] = {0x51,0x2A,0x11,0xF0,0x03};
static char FMD_ENCODE_TAG_MUTE[5]  = {0x51,0x2A,0x10,0x01,0x01};
static char FMD_ENCODE_LOW_POWER[5] = {0x51,0x2A,0x21,0x00,0x00};
#define OLEDSCREEN 1

static char dummy[8]      ={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
char data[125];
class GW :public SX1278Interface
{
public:
  GW()=default;
  ~GW()=default;
 void TxDone(void)override{
   TracePrinter::printTrace("GW:  TxDone()\n");
 }
 void TxTimeout(void)override{
TracePrinter::printTrace("GW:  TxTimeout()\n");
 }
  void RxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)override{
Serial.printf("GW:  RxDone:%d\n",size);
Serial.printf("GW:  RxDone:%.*s\n",size,payload);
  }
 void RxTimeout(void)override{
// sx1278.SX1278SetStandby();
 sx1278.SX1278SetRx(3000);
Serial.println("GW:  RxTimeout()\n");
 }
 void RxError(void)override{
Serial.println("GW:  RxError()\n");
 //  sx1278.SX1278SetStandby();
 }
 void FhssChangeChannel(uint8_t currentChannel)override{
Serial.println("GW:  FhssChangeChannel()\n");
 }
 void CadDone(bool channelActivityDetected)override{
Serial.println("GW:  CadDone()\n");
 }
 void CadDetected ()override{
Serial.println("GW:  CadDetected()\n");
 }
};


char testData[]="11111111111111111111111111111111111111111111111111111111112222222222222222222222222222222222222222222222222222222222";

void setup() {
 // put your setup code here, to run once:
  #ifdef NDEBUG
    //WIFI Kit series V1 not support Vext control
    Heltec.begin(false, true , false , true, BAND);
    //Serial.setDebugOutput(false);
    //LoRa.dumpRegisters(Serial);
  #else
    #ifdef OLEDSCREEN
        Heltec.begin(true, false , true , true, BAND);
        //PlatformDebug::init(Serial,oled);
        PlatformDebug::getInstance()->init(Serial,OLEDScreen<12>(Heltec.display));
        PlatformDebug::printLogo();
        TracePrinter::init();
    #else
      Heltec.begin(false, false , true , true, BAND);
      PlatformDebug::getInstance()->init(Serial);
    #endif
  #endif

  
  
   
    for(;;){
      PlatformDebug::printf("BatteryTemp:\n\n");
      app::BatteryTemp::measure();
      ThisThread::sleep_for(3000);
      PlatformDebug::printf("BatteryVoltage16:\n\n");
      app::BatteryVoltage::measure(16);
      ThisThread::sleep_for(3000);

      PlatformDebug::printf("BatteryVoltage17:\n\n");
      app::BatteryVoltage::measure(17);
      ThisThread::sleep_for(3000);
    }
    
PlatformDebug::pause();
  //Button button1(36);
  
  SX1278Interface* gw=new GW();
  sx1278.addDelegateObject(&gw);
  sx1278.IoInit(SX1278_MOSI, SX1278_MISO, SX1278_SCLK, SX1278_NSS,
                SX1278_DIO0, SX1278_DIO1, SX1278_DIO2, SX1278_DIO3, NC, NC,
                SX1278_RST); 


  static bool tagSetup=false;
  static bool tagAlert=false;

  pinMode(22,INPUT);
  pinMode(23,INPUT);

  pinMode(21,OUTPUT);
  digitalWrite(21,1);

  pinMode(0,INPUT);
  pinMode(2,INPUT);
  /*
  attachInterrupt(0,[]{
    detachInterrupt(0);
    tagSetup=true;
  },FALLING);

  attachInterrupt(2,[]{
      detachInterrupt(2);
      tagAlert=true;
  },FALLING);

  while(1){
  if(tagSetup){
    tagSetup=false;
    debug("tagSetup\n");
    pinMode(22,OUTPUT);
    pinMode(23,OUTPUT);
    digitalWrite(22,1);
    digitalWrite(23,1);
    
    //transmit(21,8,FMD_ENCODE_TAG_SETUP);
    //transmit(21,4,FMD_ENCODE_TAG_SETUP);
    //transmit(21,8,FMD_ENCODE_TAG_SETUP);
     transmit(21,8,FMD_ENCODE_LOW_POWER);
    transmit(21,4,FMD_ENCODE_LOW_POWER);
    transmit(21,8,FMD_ENCODE_LOW_POWER);

    digitalWrite(23,0);
    pinMode(22,INPUT);
    pinMode(23,INPUT);

    attachInterrupt(0,[]{
      detachInterrupt(0);
        tagSetup=true;
    },FALLING);
    
  }else if(tagAlert){
    tagAlert=false;
    pinMode(22,OUTPUT);
    digitalWrite(22,1);
    debug("tagAlert\n");
    transmit(21,8,FMD_ENCODE_TAG_ALERT);
    transmit(21,4,FMD_ENCODE_TAG_ALERT);
    transmit(21,8,FMD_ENCODE_TAG_ALERT);
    pinMode(22,INPUT);

    attachInterrupt(2,[]{
      detachInterrupt(2);
        tagAlert=true;
    },FALLING);
  }else{
    ThisThread::sleep_for(1000);
  }
}*/
  try
  {
     //box->wire1_join_to_i2c_bus(32,33);
     box->platformio_init();
     //box->start();
     //box->i2c_devices_init();
    // box->lora_gateway_init();
    //box->lora_collector_init();
    //box->lora_beacon_init();
    //box->lora_tag_init();
  }catch(os::alloc_error& e){
    TracePrinter::printTrace(e.what());
    Logger::error(__FUNCTION__,__LINE__);
  }
  
  //thd.start(mbed::callback(RT));
 
}

/*
You are getting this message because your board is not correctly powered. The underlying reason could be one of many things:
The USB cable is of poor quality, or too long.
Your computer's USB port cannot supply enough power to the board.
The ESP32Cam is defective
Other components in your circuit are not correctly wired up, affecting the power supply.
I would try to power the ESP32Cam with another USB cable, a different computer, or an external 5V power supply. If all of that doesn't help, it could be that your board is broken.
*/
#define LED_PWM 23 //把调用的GPIO引脚进行了一个宏定义
int freq = 5000;
int ledChannel = 0;
int resolution = 8;
void loop() {
  
PlatformDebug::println(" ----------- IPS ----------- ");

 // uint64_t mask = 1ull << 26;
  //esp_sleep_enable_ext1_wakeup(mask,ESP_EXT1_WAKEUP_ALL_LOW);
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 1); 
 //esp_sleep_enable_timer_wakeup(200000); 
  // esp_deep_sleep_start();   
          
    //gpio_wakeup_enable(GPIO_NUM_22,GPIO_INTR_LOW_LEVEL);

  while (true) {
    switch (guard::LoopTaskGuard::getLoopTaskGuard().get_signal_id())
    {
     case 0:
          {
              box->start_web_service();
          }
       break;
     case 1:
          { 

          }
      break;
     case 2:
          {
              
          }
      break;
     case 3:
          {
            
          }
      break;
     default:
      break;
    }
  }
}

void transmit(uint8_t pin,int cnt,char* data){
 
   
   do{
     
      for(int i=0;i<6;i++){
        digitalWrite(pin,1);
        usleep(10);
        digitalWrite(pin,0);
        usleep(10);
      }

      digitalWrite(pin,1);
      usleep(200);
      digitalWrite(pin,0);
      delay(10);
      
      for(char i=0;i<5;i++){
        for(char j=0;j<8;j++){
            if( (data[i] & dummy[j]) > 0 ){
                digitalWrite(pin,1);
                usleep(990);
                digitalWrite(pin,0);
                usleep(380);
            } else{
                digitalWrite(pin,1);
                usleep(380);
                digitalWrite(pin,0);
                usleep(990);
            }
        }
      }
      digitalWrite(pin,1);
      usleep(400);

      digitalWrite(pin,0);
      usleep(400);
  }while(--cnt>0);

}