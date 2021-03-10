#ifndef TEST_H
#define TEST_H

#include <arduino.h>
#include <rtos/rtos.h>
#include <app/TimeMachine/TimeMachine.h>
//vTaskSuspend(handleTaskDebug);

extern TimeMachine<DS1307> timeMachine;


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

struct A
{
    A()=default;
    A(A& a)=default;
    A(A&& a)=default;
 
    void Fun(){   Serial.println("################### Fun ################"); }
    void Fun1(int i){ Serial.println("################### Fun1 ################" +String(i,DEC));  }
    void Fun2(int i, double j){ Serial.println("################### Fun2 ################" +String(i,DEC)+String(j,DEC)); }
    void Fun3(double&& i){ Serial.println("################### Fun2 ################" +String(i,DEC)); }
    void Fun4(String a){ Serial.println("################### Fun4 ################" +a); }
    void Fun5(String&& a){ Serial.println("################### Fun5&& ################" +a); }
    void Fun6(String&& a,String&& b,int&& c){ Serial.println("################### Fun5&& ################" +a+b+String(c,DEC)); }
};

class MyTest{
public:
  void mytest(A &&a){
    Serial.println(">>>>>>>>..."+String(sizeof(a),DEC));
    
  }
  void Fun(Callback<void(String&&,String&&,int&&)>&& callback){
    callback.call(std::move(String("A")),std::move(String("B")),1);
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


void TaskDebug( void *pvParameters );

#endif