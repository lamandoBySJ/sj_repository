#ifndef PRODUCT_STLB_H
#define PRODUCT_STLB_H

#include "Arduino.h"
#include "BH1749NUC_REG/bh1749nuc_reg.h"

enum class MeasEventType : char{
        EventSystemMeasure = 0,
        EventServerMeasure,
        EventWebAppOffset,
        EventWebAppMeasure
};

namespace product_stlb{
    
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

struct api
{
static RGBProperties& get_rgb_properties()
{
    static  RGBProperties  RGBproperties;
    return RGBproperties;
}
};
}//namesapce product_stlb

#endif