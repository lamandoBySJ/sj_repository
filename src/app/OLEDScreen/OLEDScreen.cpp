
#include "app/OLEDScreen/OLEDScreen.h"
#include "app/OLEDScreen/images.h"

//display = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_64_32);
template<int N>
bool OLEDScreen<N>::init(){
  
  _display->init();
  _display->flipScreenVertically();
  _display->setFont(ArialMT_Plain_10);
  //_display->drawString(0, 0, "OLED initial done!");
  _display->clear();
  _display->display();
  return true;
}

template<int N>
void OLEDScreen<N>::logo(){
  #if !defined(NDEBUG)
  _display -> clear();
  _display -> drawXbm(0,5,logo_width,logo_height,(const unsigned char *)logo_bits);
  _display -> display();
 // osi_Sleep(300);
 // delay(200);
  _display -> clear();
  #endif
}

template<int N> 
size_t OLEDScreen<N>::println(const String& s)
{
#if !defined(NDEBUG)

   uint16_t textLen=0;
  uint16_t textPixLen=0;
  uint16_t clipPix=0;
  String clipText="";
  String text=s;
  textVector.clear();

  do{
      textLen = text.length();
      textPixLen=_display->getStringWidth(text);
      if(textPixLen>127){
        //优化算法 通常23~26个字符需要128个像素显示一行字
        int n=textLen;
        if(textLen > 26 ){
          n=26;
        }
        clipText=text.substring(0,n);
        clipPix=_display->getStringWidth(clipText);

        if(clipPix > 126){
          do{
            clipText=text.substring(0,--n);
            clipPix=_display->getStringWidth(clipText);
          }while(clipPix >126);
           //Serial.println("-->>>>>>>>>>>>>>"+String(n,DEC));
        }else{
          do{
            clipText=text.substring(0,++n);
            clipPix=_display->getStringWidth(clipText);
          }while(clipPix<126);
         // Serial.println("++>>>>>>>>>>>>>>"+String(n,DEC));
        }
        /*int n=0;
        do{
            clipText=text.substring(0,++n);
            clipPix=display->getStringWidth(clipText);
            Serial.println(">>>>>>>>>>>>>>"+String(n,DEC));
        }while(clipPix<126);
        */
        textVector.push_back(String(clipText));
        text=text.substring(n,textLen);
      }else{
           textVector.push_back(String(text));
           textPixLen = 0;
      }
      
  }while(textPixLen>0);

  for(auto& v : textVector){
      printScreen(v.c_str());
     // Serial.println("v:"+v);
  }     
  return 0;  
#else
    return 0;
#endif
}

template<int N>
void OLEDScreen<N>::printScreen(const char* data)
{
    #if !defined(NDEBUG)

    char py_pos=50;
    int text_pos=_head;
    this->_display->clear();
    if((_head-_tail)>=0){
      
        _pool[_head]=data;
        
        if((_head-_tail)<5){
            for(unsigned char i=0;i<6;++i){
               _display -> drawString(0, i*10, _pool[i]);
               delay(1);
            }
            
        }else{
          
            for(unsigned char i=0;i<6;++i){
             _display -> drawString(0, py_pos, _pool[text_pos-i]);
              delay(1);
               py_pos-=10;
            }
            ++_tail;
        }
        ++_head;
        
    }else if((_head-_tail)<0){
      
        _pool[_head]=data;
       
        int n=_head;

        for(int i=0;i<=n;++i){
          _display -> drawString(0, py_pos, _pool[text_pos].c_str());
          delay(1);
          py_pos-=10;
          --text_pos;
        }
        
        text_pos= _tail;
        n=6-_head-1;
        py_pos=0;
        
        for(int i=0;i<n;++i){
           _display -> drawString(0, py_pos, _pool[text_pos].c_str());
            delay(1);
            py_pos+=10;
            ++text_pos;
        }
        ++_tail;
        ++_head;
    }
 
  
    _tail %= N;
    _head %= N;
 
    _display -> display();

    #endif
}


template class OLEDScreen<12>;