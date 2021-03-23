#include "Test.h"

//TickerDataClock clock(NULL);
 //TickerDataClock::time_point today =  clock.now();
 //time_t tt= today.time_since_epoch().count();

//static system_clock::time_point today = system_clock::now();
// std::time_t tt = system_clock::to_time_t(today);
//debug( "today is:%s\n ",ctime(&tt));

//thread1.start(callback(TaskTest0));
//thread2.start(callback(TaskTest,&a));
typedef struct {
    float    voltage; /* AD result of measured voltage */
    float    current; /* AD result of measured current */
    uint32_t counter; /* A counter value               */
    String message;
    //char data[128];
} mail_t;

typedef struct {
    float    voltage;   /* AD result of measured voltage */
    float    current;   /* AD result of measured current */
    uint32_t counter;   /* A counter value               */
} message_t;

MemoryPool<message_t, 6> mpool;
rtos::Queue<message_t,6> queue;
rtos::Mail<mail_t, 2> mail_box;


void send_thread(void)
{
    uint32_t i = 0;
    while (true) {
        i++; // fake data update
        message_t *message = mpool.alloc();
        message->voltage = (i * 0.1) * 33;
        message->current = (i * 0.1) * 11;
        message->counter = i;
        osStatus  status = queue.put(message);
        ThisThread::sleep_for(100);
    }
  vTaskDelete(NULL);
}

void send_thread_mail(void)
{
    uint32_t i = 0;
    while (true) {
        i++; // fake data update
        mail_t *mail = mail_box.alloc();
        mail->voltage = (i * 0.1) * 33;
        mail->current = (i * 0.1) * 11;
        mail->counter = i;
        mail_box.put(mail);
        mail->message=String("mail message: "+String(i,DEC));
        ThisThread::sleep_for(100);
    }
  vTaskDelete(NULL);
} 

void Test::run_test()
{
  String topic;
  String payload;
      while(true){
        osEvent evt= _mail_box.get();
        if (evt.status == osEventMail) {
            test::mail_t *mail = (test::mail_t *)evt.value.p;
            topic = mail->topic;
            payload = mail->payload;
            _mail_box.free(mail); 
            for(auto& v : _debugCallbacks){
                v.call(mail->topic,mail->payload);
            }
            
        }
      }
}
/*attachInterrupt(22, []  {
        detachInterrupt(22);
        uint32_t i = 0;
        i++; 
        mail_t *mail = mail_box.alloc();
        mail->voltage = (i * 0.1) * 33;
        mail->current = (i * 0.1) * 11;
        mail->counter = i;
        mail->message=String("mail message: "+String(i,DEC));
        mail_box.put_from_isr(mail);
    }, RISING);   
  
  
void loop(){
    static uint32_t cnt = 0;
    static uint32_t i = 0;
    while (true) {
        osEvent evt = mail_box.get();
        if (evt.status == osEventMail) {
            mail_t *mail = (mail_t *)evt.value.p;
            Serial.printf("\nVoltage: %.2f V\n\r", mail->voltage);
            platform_debug::PlatformDebug::printf("Voltage: %.2f V",mail->voltage);
            Serial.printf("Current: %.2f A\n\r", mail->current);
            platform_debug::PlatformDebug::printf("Current: %.2f A",mail->current);
            Serial.printf("Number of cycles: %lu\n\r", mail->counter);
            platform_debug::PlatformDebug::printf("Number of cycles:%lu",mail->counter);
            platform_debug::PlatformDebug::println(mail->message);

            mail_box.free(mail);
        } 
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(100));
  }
}
        osEvent evt = queue.get();
        if (evt.status == osEventMessage) {
            message_t *message = (message_t *)evt.value.p;
            Serial.printf("\nVoltage: %.2f V\n\r", message->voltage);
            Serial.printf("Current: %.2f A\n\r", message->current);
            Serial.printf("Number of cycles: %u\n\r", message->counter);
            mpool.free(message);
        }
         Serial.println(String(++cnt,DEC)+"___________________________________"+String(evt.status,DEC));
        */
//void TaskDebug( void *pvParameters );
