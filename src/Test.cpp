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

namespace thread_test{
  /*
static bool run=true;
static std::mutex foo, bar;
static std::mutex mtx;   
  void task_a()
{
    std::lock(foo, bar);         // simultaneous lock (prevents deadlock)
    std::unique_lock<std::mutex> lck1(foo, std::adopt_lock);
    std::unique_lock<std::mutex> lck2(bar, std::adopt_lock);
      //std::cout << "task a\n";
        Serial.println("task a\n");
    while (run)
    {
       std::this_thread::sleep_for(chrono::seconds(1));
       Serial.println("task a\n");
      // (unlocked automatically on destruction of lck1 and lck2)
    }
}
void task_b()
{
  while (1)
  {
      // unique_lock::unique_lock: Constructs a unique_lock
    // foo.lock(); bar.lock(); // replaced by:
    std::unique_lock<std::mutex> lck1, lck2;
    lck1 = std::unique_lock<std::mutex>(bar, std::defer_lock);
    lck2 = std::unique_lock<std::mutex>(foo, std::defer_lock);
    std::lock(lck1, lck2);       // simultaneous lock (prevents deadlock)
    //std::cout << "task b\n";
    Serial.println("task b\n");
    // (unlocked automatically on destruction of lck1 and lck2)
  }
}
int test_unique_lock_1()
{
	std::thread th1(task_a);
	std::thread th2(task_b);

	th1.join();
	th2.join();

	return 0;
}


 
void print_thread_id(int id) {
	std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
	// critical section (exclusive access to std::cout signaled by locking lck):
	// unique_lock::lock: Calls member lock of the managed mutex object.
	lck.lock();
	Serial.println(String("thread #")+ String(id,DEC));
	// unique_lock::unlock: Calls member unlock of the managed mutex object, and sets the owning state to false
	lck.unlock();
}
void print_thread_id_2(int id) {
	std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
	// critical section (exclusive access to std::cout signaled by locking lck):
	// unique_lock::lock: Calls member lock of the managed mutex object.
	lck.lock();
	Serial.println(String("2thread #")+ String(id,DEC));
	// unique_lock::unlock: Calls member unlock of the managed mutex object, and sets the owning state to false
	lck.unlock();
}

void print_thread_id_test(int id) {
	std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
	// critical section (exclusive access to std::cout signaled by locking lck):
	// unique_lock::lock: Calls member lock of the managed mutex object.
	lck.lock();
	Serial.println(String("thread #")+ String(id,DEC));
	// unique_lock::unlock: Calls member unlock of the managed mutex object, and sets the owning state to false
//	lck.unlock();
while(run){std::this_thread::sleep_for(chrono::seconds(1));}
}

void print_thread_id_test2(int id) {
	std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
	// critical section (exclusive access to std::cout signaled by locking lck):
	// unique_lock::lock: Calls member lock of the managed mutex object.
	lck.lock();
	Serial.println(String("2thread #")+ String(id,DEC));
	// unique_lock::unlock: Calls member unlock of the managed mutex object, and sets the owning state to false
	//lck.unlock();

}
int test_unique_lock_cd()
{
std::thread threads[10];
std::thread threads2[10];
	// spawn 10 threads:
	for (int i = 0; i<10; ++i){
	    threads[i] = std::thread(print_thread_id, i + 1);
    	threads2[i] = std::thread(print_thread_id_2, i + 1);
  }
	
	for (auto& th : threads) th.join();
	for (auto& th : threads2) th.join();
	return 0;
}*/
}
