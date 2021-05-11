#include "platform_debug.h"

using namespace platform_debug;


#if !defined(NDEBUG)
PlatformDebug* PlatformDebug::_platformDebug=nullptr;
std::mutex  PlatformDebug::_mtx;
bool PlatformDebug::_finished=false;
TracePrinter* TracePrinter::_tracePrinter=nullptr;
std::mutex TracePrinter::_mtx;
#endif