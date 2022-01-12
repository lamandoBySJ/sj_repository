#ifndef LED_INDICATOR
#define LED_INDICATOR
#include "platform_debug.h"

enum class LedName:uint8_t
{
    WiFi=0,MQTT,RTC,ALS,WEB,SYS
};

class LED
{
public:
    LED()=delete;
    ~LED()=delete;

    static void io_state_reset();
    static void io_state(LedName name,bool ok);

private:
    rtos::Mutex _mtx;
};

#endif