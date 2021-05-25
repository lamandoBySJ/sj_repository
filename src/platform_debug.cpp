#include "platform_debug.h"


void TracePrinter::println(const String& e){
        #if !defined(NDEBUG)
        std::lock_guard<rtos::Mutex> lck(_mtx);
        mail_trace_t *mail = _mail_box.alloc();
        mail->log = e;
        _mail_box.put(mail);
        #endif
}





