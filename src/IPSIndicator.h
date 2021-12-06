#pragma once

#include "platform_debug.h"


class IPSIndicator
{
    public:
        IPSIndicator();
        void io_state_reset();
        void io_state_wifi(bool ok);
        void io_state_mqtt(bool ok);
        void io_state_rtc(bool ok);
        void io_state_als(bool ok);
        void io_state_web(bool ok);
        void io_state_sys(bool ok);
        static IPSIndicator& getLEDIndicator(){
            static IPSIndicator ipsIndicator;
            return ipsIndicator;
        }
    private:
        rtos::Mutex _mtx;
};
