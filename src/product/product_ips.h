#ifndef PRODUCT_IPS_H
#define PRODUCT_IPS_H

#include "Arduino.h"
namespace platform_ips{
struct IPSProtocol
{
    IPSProtocol(){
        technology="LoRa";
        family="k49a";
        gateway="9F8C";
        collector="9F8C";
        mode = "learn";
    }
    String technology;
    String family;
    String gateway;
    String collector;
    String mode;
};
struct api
{
static IPSProtocol& get_ips_properties()
{
    static  IPSProtocol  ipsProperties;
    return ipsProperties;
}
};
}//namespace product_ips
#endif