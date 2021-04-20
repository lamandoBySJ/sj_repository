
#include "app/OLEDScreen/OLEDScreen.h"
#include "app/OLEDScreen/images.h"


//#define NDEBUG
//OLEDScreen<std::string>* OLEDScreen<T>::_screen=new OLEDScreen<std::string>(12);
template<int N>
rtos::Mutex OLEDScreen<N>::_mutex;

template<int N>
OLEDScreen<N>::OLEDScreen(SSD1306Wire* ssd1306)
{
  _head=0;
  _tail=0;
  this->display = ssd1306;
  //display = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_64_32);
}
template<int N>
bool OLEDScreen<N>::init(){
  
  if(!display->init()){
    return false;
  }
      #if !defined(NDEBUG)
      display->flipScreenVertically();
      display->setFont(ArialMT_Plain_10);
      display->drawString(0, 0, "OLED initial done!");
      display->display();
      #endif
  return true;
}
template<int N>
OLEDScreen<N>::~OLEDScreen()
{
    delete display;
}
template<int N>
void OLEDScreen<N>::logo(){
  #if !defined(NDEBUG)
  display -> clear();
  display -> drawXbm(0,5,logo_width,logo_height,(const unsigned char *)logo_bits);
  display -> display();
 // osi_Sleep(300);
  delay(200);
  display -> clear();
  #endif
}
/*
template<int N> 
void OLEDScreen<N>::printf(const char* tag,const char*  format, ...)
{
  #if !defined(NDEBUG)
      va_list args;
      va_start(args, format);
      vsnprintf(_buf, sizeof(_buf), format, args);
      va_end(args);
      print(String(tag)+String(":")+String(_buf));
   #endif
}
template<int N>
void OLEDScreen<N>::printf(const char* tag,const String& data)
{
  #if !defined(NDEBUG)
    snprintf(_buf,sizeof(_buf),"%s:%s",tag,data.c_str());
    print(String(_buf));
  #endif
}


template<int N>
void OLEDScreen<N>::println(const String& data)
{
 // #if !defined(NDEBUG)
 //   print(data);
 //   println();
//  #endif
}
*/

template<int N> 
size_t OLEDScreen<N>::printf(const char *format, ...) 
{
#if !defined(NDEBUG)
    char loc_buf[64];
    char * temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
    va_end(copy);
    if(len < 0) {
        va_end(arg);
        return 0;
    };
    if(len >= sizeof(loc_buf)){
        temp = (char*) malloc(len+1);
        if(temp == NULL) {
            va_end(arg);
            return 0;
        }
        len = vsnprintf(temp, len+1, format, arg);
    }
    va_end(arg);
    //len = write((uint8_t*)temp, len);
    print(temp);
    if(temp != loc_buf){
        free(temp);
    }
    return len;
    
#else
    return 0;
#endif
}
template<int N> 
size_t OLEDScreen<N>::println(const String& s)
{
#if !defined(NDEBUG)
    print(s+String("\n"));
    return 0;
#else
    return 0;
#endif
}
template<int N>
void OLEDScreen<N>::print(const String& data)
{

  //display ->getStringWidth(text);
  //display ->width();
  _mutex.lock();
  //uint16_t remainPix=0;
  uint16_t textLen=0;
  uint16_t textPixLen=0;
  uint16_t clipPix=0;
  String clipText="";
  String text=data;
  textVector.clear();

  do{
      textLen = text.length();
      textPixLen=display->getStringWidth(text);
      if(textPixLen>127){
        //优化算法 通常23~26个字符需要128个像素显示一行字
        int n=textLen;
        if(textLen > 26 ){
          n=26;
        }
        clipText=text.substring(0,n);
        clipPix=display->getStringWidth(clipText);

        if(clipPix > 126){
          do{
            clipText=text.substring(0,--n);
            clipPix=display->getStringWidth(clipText);
          }while(clipPix >126);
           //Serial.println("-->>>>>>>>>>>>>>"+String(n,DEC));
        }else{
          do{
            clipText=text.substring(0,++n);
            clipPix=display->getStringWidth(clipText);
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
      //Serial.println(String(v.length(),DEC)+String(" : ")+String(display->getStringWidth(v),DEC));
  }     
  _mutex.unlock();
}
/*
template<int N>
void OLEDScreen<N>::print(String&& s)
{

  //display ->getStringWidth(text);
  //display ->width();
  _mutex.lock();
 
  uint16_t textLen=0;
  uint16_t remainPix=0;
  uint16_t clipPix=0;
  String clipText="";
  String text=s;
  textVector.clear();

  do{
      int n=0;
      textLen=display->getStringWidth(text);
      if(textLen>128){
          remainPix=textLen-128;
          do{
            clipText=text.substring(text.length()-(++n));
            clipPix=display->getStringWidth(clipText);
          }while(remainPix>clipPix);
          
          textVector.push_back(text.substring(0,text.length()-n));
          text=clipText;
          Serial.println(">>>"+String(text.length(),DEC));
      }else{
           Serial.println("<<<<"+String(text.length(),DEC));
           textVector.push_back(String(text));
      }
      Serial.println(String(textLen,DEC));
  }while(textLen>128);

  for(auto& v : textVector){
      printScreen(v.c_str());
      Serial.println(String("&&->")+v);
  }     
  _mutex.unlock();
}*/
template<int N>
void OLEDScreen<N>::printScreen(const char* data)
{
    #if !defined(NDEBUG)

    char py_pos=50;
    int text_pos=_head;
    this->display -> clear();
 
    if((_head-_tail)>=0){
      
        _pool[_head]=data;
        
        if((_head-_tail)<5){
            for(unsigned char i=0;i<6;++i){
               display -> drawString(0, i*10, _pool[i]);
               delay(1);
            }
            
        }else{
          
            for(unsigned char i=0;i<6;++i){
             display -> drawString(0, py_pos, _pool[text_pos-i]);
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
          display -> drawString(0, py_pos, _pool[text_pos].c_str());
          delay(1);
          py_pos-=10;
          --text_pos;
        }
        
        text_pos= _tail;
        n=6-_head-1;
        py_pos=0;
        
        for(int i=0;i<n;++i){
           display -> drawString(0, py_pos, _pool[text_pos].c_str());
            delay(1);
            py_pos+=10;
            ++text_pos;
        }
        ++_tail;
        ++_head;
    }
 
  
     _tail %= N;
    _head %= N;
 
    display -> display();

    #endif
}

template<int N>
void OLEDScreen<N>::screenPrint(const char* data)
{
    #if !defined(NDEBUG)

    char py_pos=50;
    int px_pos=_head;
    this->display -> clear();
 
    if((_head-_tail)>=0){
      
        _pool[_head]=data;
        
        if((_head-_tail)<5){
            for(unsigned char i=0;i<6;++i){
               display -> drawString(0, i*10, _pool[i]);
               delay(1);
            }
            
        }else{
          
            for(unsigned char i=0;i<6;++i){
             display -> drawString(0, py_pos, _pool[px_pos-i]);
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
          display -> drawString(0, py_pos, _pool[px_pos].c_str());
          delay(1);
          py_pos-=10;
          --px_pos;
        }
        
        px_pos= _tail;
        n=6-_head-1;
        py_pos=0;
        
        for(int i=0;i<n;++i){
           display -> drawString(0, py_pos, _pool[px_pos].c_str());
            delay(1);
            py_pos+=10;
            ++px_pos;
        }
        ++_tail;
        ++_head;
    }
 
  
     _tail %= N;
    _head %= N;
 
    display -> display();

    #endif
}
template class OLEDScreen<12>;

/*
OLEDScreen* OLEDScreen::getTerminal()
{
  if( xSemaphoreTake(OLEDScreen:: xSemaphore, ( TickType_t ) portMAX_DELAY) == pdTRUE ){
   
    if( OLEDScreen::_screen==NULL){
       OLEDScreen::_screen=new OLEDScreen(12);
       
    }
    xSemaphoreGive( xSemaphore);
 }
  return OLEDScreen::_screen;
}*/
