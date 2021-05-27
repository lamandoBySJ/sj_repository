#ifndef LED_INDICATOR
#define LED_INDICATOR
#include "platform_debug.h"
class LEDIndicator
{
public:
    LEDIndicator();
    void io_state_reset();
    void io_state_wifi(bool ok);
    void io_state_mqtt(bool ok);
    void io_state_rtc(bool ok);
    void io_state_als(bool ok);
    void io_state_web(bool ok);
    void io_state_sys(bool ok);
    static LEDIndicator& getLEDIndicator(){
       static LEDIndicator ledIndicator;
       return ledIndicator;
    }
private:
    rtos::Mutex _mtx;
};

#endif