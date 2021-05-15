#include "platform_debug.h"

using namespace platform_debug;

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


#if !defined(NDEBUG)
PlatformDebug* PlatformDebug::_platformDebug=nullptr;
std::mutex  PlatformDebug::_mtx;
bool PlatformDebug::_finished=false;
TracePrinter* TracePrinter::_tracePrinter=new TracePrinter();
std::mutex TracePrinter::_mtx;
#endif