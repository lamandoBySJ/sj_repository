#ifndef OLED_SCREEN_H
#define OLED_SCREEN_H

#include "Arduino.h"
#include <Wire.h>
#include "oled/SSD1306Wire.h"
#include "oled/OLEDDisplayFonts.h"
#include "rtos/rtos.h"
#include <thread>
#include <mutex>
using namespace std;
/*
使用一个私有函数初始化成员变量
使用带有默认参数的构造函数
使用placement new运算符调用重载构造函数
使用C++11的委托构造函数（在初始化列表位置调用）
*/
template<int N=12>
class OLEDScreen 
{
public:
    OLEDScreen():_head(0),_tail(0),_display(nullptr)
    {

    }
    OLEDScreen(SSD1306Wire* ssd1306):_head(0),_tail(0),_display(ssd1306)
    {

    }
    ~OLEDScreen()
    {
        if(_display){
          delete _display;
          _display=nullptr;
        }
    }
    OLEDScreen(const OLEDScreen& other)
    {
      if (this != &other) {
        if (other._display != nullptr){
             /*
             this-> _display = new SSD1306Wire(other._display->_address
             ,other._display->_sda
             ,other._display->_scl
             ,other._display->_rst
             ,other._display->geometry);*/
             this-> _display=other._display;
            _head=other._head;
            _tail=other._tail;
          // textVector.clear();
        }else{
            generate();
        }   
      }
    }
    OLEDScreen(OLEDScreen&& other):OLEDScreen()
    {   
     //new (this)OLEDScreen();
      if (this != &other) {
          if (other._display != nullptr){
              this->~OLEDScreen();
              this->_display = other._display;
              other._display = nullptr;
              this->_head=other._head;
              this->_tail=other._tail;

          }else{
              generate();
          }
      }
    }

  OLEDScreen&  operator=(const OLEDScreen& other)
    {
      if (this != &other) {
        if(other._display!=nullptr){
            this-> _display=other._display;
            _head=other._head;
            _tail=other._tail;
        }else{
              generate();
        }
      }
      return *this;
    }

 OLEDScreen&  operator=(OLEDScreen&& other)
  {
      if (this != &other){
            this->~OLEDScreen();
            if(other._display!=nullptr){
              this->_display  = other._display ;
              other._display  = nullptr;
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
 // size_t printf(const char *format, ...);

protected:
  void generate(){
      this-> _display = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);
      _head=0;
      _tail=0;
  }
 // void print(const String& data);
  void printScreen(const char* data);
  //void screenPrint(const char* data);
  
private:
  String _pool[N];
  std::vector<String> textVector;
  int _head;
  int _tail;
  SSD1306Wire* _display;
};

#endif
