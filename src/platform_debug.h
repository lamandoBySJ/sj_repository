#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#include "arduino.h"
#include "app/OLEDScreen/OLEDScreen.h"
#include "platform/Callback.h"

//#define NDEBUG

namespace platform_debug
{

//template <typename Signature>
//class PlatformDebug;
 inline  namespace show
{
template<typename Tuple, int N = std::tuple_size<Tuple>::value>
struct Printer
{
  static void log(Tuple& t,const char* data) {
    Printer<Tuple, N - 1>::log(t);
  
    using type = typename std::tuple_element<N - 1, Tuple>::type;
  //  std::string ts = typeid(type).name();
    type& v = std::get<N - 1>(t);
  
   // v.println(data);
  }
};
  
template<typename Tuple>
struct Printer<Tuple,  1>
{
  static void log(Tuple& t,const char* data) {
    using type = typename std::tuple_element<0, Tuple>::type;
    //std::string ts = typeid(type).name();
    type& v = std::get<0>(t);
    Serial.println("11111111111111111111111");
    v.println(data);
  }
};
template<typename Tuple>
struct Printer<Tuple,  0>
{
  static void log(Tuple& t,const char* data) {
    Serial.println("00000000000000000000000000000");
   // type& v = std::get<0>(t);
    //v.println(data);
  }
};


inline namespace show2
{
template<class Tuple, std::size_t N>
struct TuplePrinter {
 static void print(const Tuple& t) 
 {
  TuplePrinter<Tuple, N-1>::print(t);
  //std::cout << ", " << std::get<N-1>(t);
  std::get<N-1>(t).println("AAA");
 }
};
 
template<class Tuple>
struct TuplePrinter<Tuple, 1> {
 static void print(const Tuple& t) 
 {
    //std::cout << std::get<0>(t);
     std::get<0>(t).println("AAA");
 }
};
 
template<typename... Args, std::enable_if_t<sizeof...(Args) == 0, int> = 0>
void print(const std::tuple<Args...>& t)
{
    //std::cout << "()\n";

}
 
template<typename... Args, std::enable_if_t<sizeof...(Args) != 0, int> = 0>
void print(const std::tuple<Args...>& t)
{
    //std::cout << "(";
    TuplePrinter<decltype(t), sizeof...(Args)>::print(t);
   // std::cout << ")\n";
}
}

} 

//template <typename Signature>
//class PlatformDebug;
class PlatformDebug
{
public:
    PlatformDebug()=default;
    ~PlatformDebug()=delete;
    /*PlatformDebug(ArgTs...args)
    { 
        static int cnt = 0;
        static int cnt2 = sizeof...(ArgTs);
        Serial.println(cnt2);
        if(std::is_same<HardwareSerial,T>::value){
             Serial.println("::is_same<HardwareSerial");
        }else if(std::is_same<TwoWire,T>::value){
                Serial.println("::is_same<TwoWire");
        }else if(std::is_same<OLEDScreen<12>,T>::value){
                Serial.println("::is_same<OLEDScreen<12>");
        }else{
                Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
        }
        
        
        if(--cnt2>=0){
          //  _objs(std::forward<ArgTs>(args)...)
         
            PlatformDebug(std::forward<ArgTs>(args)...);
        }
    }
    */
    template <class U,class...Args>
    static void init(U&& u,Args&&...args)//:_objs(std::forward<ArgTs>(args)...)
    {
        #if !defined(NDEBUG)
        if(PlatformDebug::_finished){
              return;
        }else if(PlatformDebug::_platformDebug==nullptr){
            PlatformDebug::_platformDebug = new PlatformDebug(); 
        }

            if(std::is_same<HardwareSerial, U>::value){
                    Serial.println("::is_same<HardwareSerial");
            }else if(std::is_same<TwoWire, U>::value){
                    Serial.println("::is_same<TwoWire");
            }else if(std::is_same<OLEDScreen<12>, U>::value){
                    Serial.println("::is_same<OLEDScreen<12>");
            }else if(std::is_same< Print , U>::value){
                    Serial.println("::is_same<Print>"); 
            }else{
                    Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
            }
            //Callback<void(const char*)>(&u,&U::println);
            PlatformDebug::_platformDebug->_call.push_back(Callback<void(const char*)>(&u,&U::printf));
            init(std::forward<Args>(args)...);
        #endif
    }
    static void inline init(){
      #if !defined(NDEBUG)
      PlatformDebug::_finished=true;
      Serial.println("#################### INIT DONE ####################");
      #endif
    }
   /* 
    template <class U,class...Args>
    PlatformDebug(U&& u,Args&&...args)//:_objs(std::forward<ArgTs>(args)...)
    {

        static int cnt2 = sizeof...(args);
        Serial.println(cnt2);
        if(std::is_same<HardwareSerial, U>::value){
             Serial.println("::is_same<HardwareSerial");
        }else if(std::is_same<TwoWire, U>::value){
                Serial.println("::is_same<TwoWire");
        }else if(std::is_same<OLEDScreen<12>, U>::value){
                Serial.println("::is_same<OLEDScreen<12>");
                callback(u,&U::println);
        }else{
                Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
        }
        
        //_call.push_back();
        Serial.println(":::::::::::::::::::::::::::::::::::::::"+String(_call.size(),DEC));
        if(--cnt2>=0){
            PlatformDebug(std::forward<Args>(args)...);
        }
    }
    PlatformDebug(T&& t)
    {
        if(std::is_same<HardwareSerial,T>::value){
             Serial.println("T::is_same<HardwareSerial");
        }else if(std::is_same<TwoWire,T>::value){
                Serial.println("T::is_same<TwoWire");
        }else if(std::is_same<OLEDScreen<12>,T>::value){
                Serial.println("T::is_same<OLEDScreen<12>");
        }else{
                Serial.println("T::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
        }
    }*/


    static inline void print(const char* data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        Serial.println(data);
        for(auto v:PlatformDebug::_platformDebug->_call){
            v.call(data);
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }

    static inline void print(const String& data) 
    {
        #if !defined(NDEBUG)
        PlatformDebug::_platformDebug->std_mutex.lock();
        Serial.println(data);
        for(auto v:PlatformDebug::_platformDebug->_call){
            v.call(data.c_str());
        }
        PlatformDebug::_platformDebug->std_mutex.unlock();
        #endif
    }
     /*template <class U,class...ArgTs>
    void printArgs(U u,ArgTs...args) {
      // x.printf("x","x");
      u.println("************From T println**************");  
      if(std::is_same<HardwareSerial,T>::value){
             Serial.println("::is_same<HardwareSerial");
      }else if(std::is_same<TwoWire,T>::value){
             Serial.println("::is_same<TwoWire");
      }else if(std::is_same<OLEDScreen<12>,T>::value){
             Serial.println("::is_same<OLEDScreen<12>");
      }else{
             Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
      }

      print(std::forward<ArgTs>(args)...);
    }
  
    template <class U,class...ArgTs>
    void printArgs(U u,ArgTs...args) {
      // x.printf("x","x");
      t.println("************From T println**************");  
      if(std::is_same<HardwareSerial,T>::value){
             Serial.println("::is_same<HardwareSerial");
      }else if(std::is_same<TwoWire,T>::value){
             Serial.println("::is_same<TwoWire");
      }else if(std::is_same<OLEDScreen<12>,T>::value){
             Serial.println("::is_same<OLEDScreen<12>");
      }else{
             Serial.println("::is_same<XXXXXXXXXXXXXXXXXXXXXXX>");
      }

      print(std::forward<ArgTs>(args)...);
    } */

private:
    static PlatformDebug* _platformDebug;
    std::vector<Callback<void(const char*)>> _call;
    static rtos::Mutex std_mutex;
    static bool _finished;
};

PlatformDebug* PlatformDebug::_platformDebug=nullptr;
rtos::Mutex  PlatformDebug::std_mutex;
bool PlatformDebug::_finished=false;
//std::vector<Callback<void(const char*)>>  PlatformDebug::_call;

} // namespace platform_debug


#endif
