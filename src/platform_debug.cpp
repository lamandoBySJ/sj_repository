#include "platform_debug.h"
#include "app/OLEDScreen/OLEDScreen.h"

using namespace platform_debug;

void TracePrinter::println(const String& e){
        #if !defined(NDEBUG)
        std::lock_guard<rtos::Mutex> lck(_mtx);
        mail_trace_t *mail = _mail_box.alloc();
        mail->log = e;
        _mail_box.put(mail);
        #endif
}





