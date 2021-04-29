#ifndef OLED_SCREEN_H
#define OLED_SCREEN_H

#include "Arduino.h"
#include <Wire.h>
#include "oled/SSD1306Wire.h"
#include "oled/OLEDDisplayFonts.h"
#include "rtos/rtos.h"

using namespace std;

template<int N=12>
class OLEDScreen 
{
public:
    OLEDScreen()=default;
    OLEDScreen(SSD1306Wire* ssd1306);
    ~OLEDScreen();
    OLEDScreen(const OLEDScreen& other)
    {
      Serial.println("copy construct");
      if (this != &other) {
        if (other.display != nullptr){
            this->~OLEDScreen();
            this->display = other.display;
            _head=other._head;
            _tail=other._tail;
          // textVector.clear();
        }else{
            generate();
        }   
      }
    }
    OLEDScreen(OLEDScreen&& other)
    {
      Serial.println("move construct");
      if (this != &other) {
          if (other.display != nullptr){
              this->~OLEDScreen();
              this->display = other.display;
              other.display = nullptr;
              _head=other._head;
              _tail=other._tail;
             // textVector.clear();
          }else{
               generate();
          }
      }
    }

    OLEDScreen& operator=(const OLEDScreen& other)
    {
      Serial.println("assign=&");
      if (this != &other) {
        this->~OLEDScreen();
        if(other.display!=nullptr){
             display = other.display;
             _head=other._head;
              _tail=other._tail;
        }else{
              generate();
        }
      }
      return *this;
    }

  OLEDScreen& operator=(OLEDScreen&& other)
  {
      Serial.println("move=&&");
      if (this != &other){
            this->~OLEDScreen();
            if(other.display!=nullptr){
              this->display  = other.display ;
              other.display  = nullptr;
              _head=other._head;
              _tail=other._tail;
            }else{
               generate();
            }
            
      }
       return *this;
   }
   void logo();
   bool init();

  size_t println(const String &s);
  size_t printf(const char *format, ...);
protected:
  void generate(){
      this-> display = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);
      _head=0;
      _tail=0;
  }
private:
  SSD1306Wire* display=nullptr;

  void print(const String& data);
  void printScreen(const char* data);
  void screenPrint(const char* data);
  String _pool[N];
  int _head;
  int _tail;

  static rtos::Mutex _mutex;
  std::vector<String> textVector;
};

#endif
