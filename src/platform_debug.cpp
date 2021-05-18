#include "platform_debug.h"
#include "app/OLEDScreen/OLEDScreen.h"

using namespace platform_debug;

#if !defined(NDEBUG)
PlatformDebug* volatile PlatformDebug::_platformDebug=nullptr;
std::mutex  PlatformDebug::_mtx;
TracePrinter* volatile TracePrinter::_tracePrinter=nullptr;
#endif

DeviceInfo Platform::deviceInfo;

void TracePrinter::println(const String& e){
        #if !defined(NDEBUG)
        std::lock_guard<rtos::Mutex> lck(_lck_mtx);
        mail_trace_t *mail = _mail_box.alloc();
        mail->log = e;
        _mail_box.put(mail);
        #endif
}

void platform_debug_init(bool SerialEnabled,bool OLEDEnabled)
{
    #if !defined(NDEBUG)
    if(SerialEnabled&& OLEDEnabled){
        // PlatformDebug::init(Serial,oled);
        PlatformDebug::init(Serial,OLEDScreen<12>(Heltec.display));
        // PlatformDebug::init(Serial,std::move(oled));
        PlatformDebug::printLogo();
    }else if(SerialEnabled){
        PlatformDebug::init(Serial);
    }else if(OLEDEnabled){
        PlatformDebug::init(OLEDScreen<12>(Heltec.display));
    }
    #endif
}






