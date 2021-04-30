#include "platform_debug.h"

using namespace platform_debug;

PlatformDebug* PlatformDebug::_platformDebug=nullptr;
rtos::Mutex  PlatformDebug::std_mutex;
bool PlatformDebug::_finished=false;
#if !defined(NDEBUG)
TracePrinter* TracePrinter::_tracePrinter;
std::mutex TracePrinter::_mtx;
#endif