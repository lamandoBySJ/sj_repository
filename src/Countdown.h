#pragma once

#include "platform_debug.h"
#include "platform/mbed.h"
#include <set>
#include "Arduino.h"
#include <atomic>

class Counter
{
public:
virtual ~Counter()=default;
virtual void count()=0;
virtual bool expired()=0;
virtual void reset()=0;
virtual time_t left_seconds()=0;
virtual const char* listener() const=0;
std::atomic<time_t> _left_seconds;
time_t _seconds;
String _listener;
};
class Countup : public Counter
{
public:
virtual  ~Countup()=default;
virtual void countup()=0;
void count() override{
  countup();
}
std::atomic<time_t> _counter;
};

class Countdown : public Counter
{
public:
virtual  ~Countdown()=default;
virtual void countdown()=0;
void count() override{
  countdown();
}
};

class CountdownListener : public Countdown
{
public:
      CountdownListener(time_t seconds=30,const char* listener="Countdown"){
        _seconds=seconds;
        _left_seconds=seconds;
        _listener=listener;
      }
virtual ~CountdownListener()=default;

void reset() override{
    _left_seconds = _seconds;
}
bool expired() override{
    return _left_seconds==0;
}
void countdown() override{
    if(--_left_seconds == 0){
        onComplete();
    }else{
        onCountdown(_left_seconds);
    }
}
time_t left_seconds() override
{
  return _left_seconds;
}
const char* listener() const override{
      return _listener.c_str();
} 

virtual void onCountdown(time_t seconds)=0;
virtual void onComplete()=0;

private:

};
class CountupListener : public Countup
{
public:
      CountupListener(time_t base_seconds=0,time_t seconds=30,const char* listener="Countup"){
        _counter=base_seconds;
        _seconds=seconds;
        _left_seconds=seconds;
        _listener=listener;
      }
virtual ~CountupListener()=default;


void reset() override{
    _left_seconds = _seconds;
}
bool expired() override{
    return _left_seconds.load()==0;
}
void countup() override{
      if(--_left_seconds == 0){
            onComplete();
      }else{
            onCountup(++_counter);
      }
}
time_t left_seconds() override
{
  return _left_seconds;
}
const char* listener() const override{
      return _listener.c_str();
} 
virtual void onCountup(time_t seconds)=0;
virtual void onComplete()=0;
time_t counter(){
  return _counter.load();
}
void syncCounter(time_t epoch){
  _counter = epoch;
}
protected:

private:

};

class TimeoutManager
{
public:
    TimeoutManager(){
      _thread.start(mbed::callback(this,&TimeoutManager::loopCounter));
    }
    void loopCounter(){
        while(true){
            ThisThread::sleep_for(1000);
            _mtx.lock();
            for(auto& v : _listeners){
              if(!v->expired()){
                 v->count();
              }
              //PlatformDebug::printf("...left_s:%d\n",v->left_seconds());
            }
            _mtx.unlock();
        }
    }
    static void countdown(Counter* listener){
      getInstance().add(listener);
    }
    static void remove(Counter* listener){
      getInstance()._remove(listener);
    }
    
  static TimeoutManager& getInstance(){
  static TimeoutManager* manager=new TimeoutManager();
  return *manager;
  }
protected:
    void _remove(Counter* listener){
      std::lock_guard<rtos::Mutex> lck(_mtx);
      _listeners.erase(listener);
    }
    void add(Counter* listener){
      if(listener->expired()){
         listener->reset();
      }else{
        std::lock_guard<rtos::Mutex> lck(_mtx);
         _listeners.insert(listener);
      }
    }
private:

  rtos::Mutex _mtx;
  Thread _thread;
  std::set<Counter*> _listeners;

};



class TimeoutCountdown : public CountdownListener
{
public:
    //TimeoutCountdown():CountdownListener(3,"default"){}
    TimeoutCountdown(const char* listener="timeout",time_t seconds=30):CountdownListener(seconds,listener){}
    TimeoutCountdown(time_t seconds=30,const char* listener="timeout"):CountdownListener(seconds,listener){}
    ~TimeoutCountdown()=default;
    void onCountdown(time_t seconds) override{
       //PlatformDebug::printf("%s:left_seconds:%d\n",listener(),seconds);
    }

    void onComplete() override{
        onTimeout();
        PlatformDebug::printf("%s: onComplete\n",listener());
    }
virtual void  onTimeout() =0;
private:

};

class TimeoutCountup : public CountupListener
{
public:
    TimeoutCountup(time_t base_seconds=0,time_t seconds=120,const char* listener="Countup"):CountupListener(base_seconds,seconds,listener){}

    ~TimeoutCountup()=default;
    void onCountup(time_t seconds) override{
       //PlatformDebug::printf("%s:onCountup:%d\n",listener(),seconds);
    }
    void onComplete() override{
        onTimeout();
        PlatformDebug::printf("%s: onComplete\n",listener());
    }
    virtual void  onTimeout() =0;

private:

};


class RestartCountdown : public CountdownListener
{
public:
    RestartCountdown():CountdownListener(30,"restart"){}
    RestartCountdown(const char* listener="restart",time_t seconds=30):CountdownListener(seconds,listener){}
    RestartCountdown(time_t seconds=30,const char* listener="restart"):CountdownListener(seconds,listener){}
    ~RestartCountdown()=default;
    void onCountdown(time_t seconds) override{
        PlatformDebug::printf("%s:left_seconds:%d\n",listener(),seconds);
    }

    void onComplete() override{
        PlatformDebug::printf("%s:onComplete:ESP.restart\n",listener());
        ThisThread::sleep_for(3000);
        ESP.restart();
    }

private:

};
