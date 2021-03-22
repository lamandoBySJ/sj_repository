#include "platform_debug.h"

using namespace platform_debug;

PlatformDebug* PlatformDebug::_platformDebug=nullptr;
rtos::Mutex  PlatformDebug::std_mutex;
bool PlatformDebug::_finished=false;
