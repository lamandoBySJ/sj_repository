#include "platform_debug.h"

using namespace platform_debug;

String DeviceInfo::BoardID="";
String DeviceInfo::Family="k49a";

#if !defined(NDEBUG)
PlatformDebug* PlatformDebug::_platformDebug=nullptr;
std::mutex  PlatformDebug::_mtx;
bool PlatformDebug::_finished=false;
TracePrinter* TracePrinter::_tracePrinter=new TracePrinter();
std::mutex TracePrinter::_mtx;
#endif