#include "platform_debug.h"
#include "app/OLEDScreen/OLEDScreen.h"

using namespace platform_debug;

#if !defined(NDEBUG)
PlatformDebug* volatile PlatformDebug::_platformDebug=nullptr;
std::mutex  PlatformDebug::_mtx;
TracePrinter* volatile TracePrinter::_tracePrinter=nullptr;
#endif

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

String DeviceInfo::BoardID="";
String DeviceInfo::Family="k49a";

String web_properties::ap_ssid="STLB_SSID";
String web_properties::ap_pass="Aa000000";
String web_properties::http_user="admin";
String web_properties::http_pass="admin";
String web_properties::server_upload_uri = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";


String rgb_properties::path ="/als_constant";
uint16_t rgb_properties::r_offset = 0;
uint16_t rgb_properties::g_offset = 0;
uint16_t rgb_properties::b_offset = 0;

String user_properties::path = "/user_constant";
String user_properties::ssid = "IoTwlan";
String user_properties::pass = "mitac1993";
String user_properties::host = "mslmqtt.mic.com.cn";
int    user_properties::port = 1883;




