#include "platform_debug.h"

using namespace platform_debug;

PlatformDebug* PlatformDebug::_platformDebug=nullptr;
std::mutex  PlatformDebug::_mtx;
bool PlatformDebug::_finished=false;
#if !defined(NDEBUG)
TracePrinter* TracePrinter::_tracePrinter;
std::mutex TracePrinter::_mtx;
#endif