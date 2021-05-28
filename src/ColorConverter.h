#ifndef COLOR_CONVERTER_H
#define COLOR_CONVERTER_H
#include "Arduino.h"
#include "ArduinoJson.h"
#include "platform_debug.h"
#include "ColorCollector.h"
#include "app/ColorSensor/ColorSensor.h"


namespace product_api
{
struct RGBProperties
{   
        RGBProperties(){
            path ="/als_constant";
            r_offset = 0;
            g_offset = 0;
            b_offset = 0;
        }
        String path;
        uint16_t r_offset;
        uint16_t g_offset;
        uint16_t b_offset;

        RGBProperties& operator=(const RGBProperties& properties){
           this->path =  properties.path;
           this->r_offset = properties.r_offset;
           this->g_offset = properties.g_offset;
           this->b_offset = properties.b_offset;
           return *this;
        }

};
RGBProperties& get_rgb_properties();
}

class ColorConverter
{
public:
    void RGBToHSL(RGB& rgb);
    bool color(RGB& rgb,JsonArray& data );
    bool convertToColor(uint16_t h,JsonArray& data);
    static ColorConverter& getColorConverter(){
        static ColorConverter colorConverter;
        return colorConverter;
    }
private:
    ColorConverter()=default;
};

#endif