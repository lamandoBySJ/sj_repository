#ifndef RTOS_H
#define RTOS_H

//#include "rtos/internal/mbed_rtos_storage.h"
#include "rtos/Kernel.h"
#include "rtos/Thread.h"
#include "rtos/ThisThread.h"
#include "rtos/Mutex.h"
#include <platform/Callback.h>
#include <platform/mbed_debug.h>
#include "platform/mbed.h"
#include "rtos/Mail.h"
#include "rtos/MemoryPool.h"
#include "rtos/EventFlags.h"
#include "rtos/internal/mbed_rtos1_types.h"
#include "rtos/Queue.h"
#include "rtos/cmsis_os2.h"
#include <cxxsupport/mstd_new.h>
//#include "rtos/Semaphore.h"
//#include "rtos/Mail.h"
//#include "rtos/MemoryPool.h"
//#include "rtos/Queue.h"
//#include "rtos/EventFlags.h"
//#include "rtos/ConditionVariable.h"


/** \defgroup rtos-public-api RTOS
 * \ingroup mbed-os-public
 */

#ifndef MBED_NO_GLOBAL_USING_DIRECTIVE
using namespace rtos;
using namespace mbed;
#endif

#endif