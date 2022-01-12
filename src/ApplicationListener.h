#pragma once 
#include "WiFiService.h"

class ApplicationListener:virtual public WiFiEventListener,virtual public MqttEventListener
{
public:
virtual ~ApplicationListener()=default;
//virtual void startup()=0;
};