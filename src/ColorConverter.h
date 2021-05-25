#ifndef COLOR_CONVERTER_H
#define COLOR_CONVERTER_H
#include "Arduino.h"
#include "ArduinoJson.h"
#include "platform_debug.h"
#include "ColorCollector.h"


class ColorConverter
{
public:
       
   
   void RGBToHSL(RGB& rgb_hsl){
        double _delta=0;
        double _cmax=0;
        double _cmin=0;
        double _r=0;
        double _g=0;
        double _b=0;
        double _h=0;
        double _s=0;
        double _l=0;

        if(rgb_hsl.R.u16bit > 255  || rgb_hsl.G.u16bit > 255 || rgb_hsl.B.u16bit > 255){
            uint16_t  base_max = max( max(rgb_hsl.R.u16bit, rgb_hsl.G.u16bit), rgb_hsl.G.u16bit);
            _r = (uint16_t )(rgb_hsl.R.u16bit*255.0 / base_max);
            _g = (uint16_t )(rgb_hsl.G.u16bit*255.0 / base_max);
            _b = (uint16_t )(rgb_hsl.B.u16bit*255.0 / base_max);  
        }else{
            _r = rgb_hsl.R.u16bit/255.0;
            _g = rgb_hsl.G.u16bit/255.0;
            _b = rgb_hsl.B.u16bit/255.0; 
        }

        _cmax = max(_r,max(_g, _b));
        _cmin = min(_r,min(_g, _b));
        _delta = _cmax - _cmin;

         
        if (_delta==0){
            _h = 0;
        }else if (_r == _cmax) {
            _h =(_g<_b ? 6.0:0)+ (_g - _b) / _delta;
        }else if (_g == _cmax){
            _h = 2.0 + (_b- _r) / _delta;
        }else{
             _h = 4.0 + (_r- _g) / _delta;
        }

        _h = _h *60;

        ////////////////////// luminance //////////////////
        _l=(_cmax+_cmin)/2.0;
        ///////////////////////// saturation ////////////////
        if(_l == 0 || _cmax == _cmin)     {
            _s = 0;
        }else if(0<_l && _l<=0.5f)    {
            _s = _delta/(_cmax+_cmin);
        }else if(_l>0.5f){
            _s = _delta/(2 - (_cmax+_cmin)); //(maxVal-minVal > 0)?
        }
            
        rgb_hsl.h = _h;
        rgb_hsl.s = _s*100;
        rgb_hsl.l = _l*100;
    }
    bool color(RGB& rgb,JsonArray& data )
    {   
        RGBToHSL(rgb);
        return convertToColor(rgb.h,data);
    }
    bool convertToColor(uint16_t h,JsonArray& data )
    {
      //JsonArray data = doc.createNestedArray("TowerColor");
      uint16_t sensor_h=h/10;
      sensor_h*=10;
        switch(sensor_h){
          case 0:{data.add("Red");}break;
          case 10:{data.add("Yellow");}break;
          case 20:{data.add("Yellow");}break;
          case 30:{data.add("Yellow");}break;
          case 40:{data.add("Yellow");}break;
          case 50:{data.add("Yellow");}break;
          case 60:{data.add("Yellow");}break;
          case 70:{data.add("Yellow");}break;
          case 80:{data.add("Yellow");}break;
          case 90:{data.add("Yellow");}break;
          case 100:{data.add("Yellow");}break;
          case 110:{data.add("Green");}break;
          case 120:{data.add("Green");}break;
          case 130:{data.add("Green");}break;
          case 140:{data.add("Green");}break;
          case 150:{data.add("Green");}break;
          case 160:{data.add("Green");}break;
          case 170:{data.add("Green");}break;
          case 180:{data.add("Green");}break;
          case 190:{data.add("Green");}break;
          case 200:{data.add("Green");}break;
          case 210:{data.add("Green");}break;
          case 220:{data.add("Blue");}break;
          case 230:{data.add("Blue");}break;
          case 240:{data.add("Blue");}break;
          case 250:{data.add("Blue");}break;
          case 260:{data.add("Blue");}break;
          case 270:{data.add("Blue");}break;
          case 280:{data.add("Blue");}break;
          
         case 290:{data.add("Blue");}break;
         case 300:{data.add("Red");}break;
         case 310:{data.add("Red");}break;
         
          case 320:{data.add("Red");}break;
          case 330:{data.add("Red");}break;
          case 340:{data.add("Red");}break;
          case 350:{data.add("Red");}break;
          case 360:{data.add("Red");}break;
          default: break;
        }

        return data.size()!=0;
    }
    static ColorConverter& getColorConverter(){
        static ColorConverter colorConverter;
        return colorConverter;
    }
private:
    ColorConverter()=default;
};

#endif