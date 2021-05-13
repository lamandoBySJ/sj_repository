#ifndef __TIMERS_H
#define __TIMERS_H

/*lint -save -e537 This headers are only multiply included if the application code
happens to also be including task.h. */
#include "rtos/tasks.h"
/*lint -restore */

#ifdef __cplusplus
extern "C" {
#endif

const char * pcTimerGetName( TimerHandle_t xTimer );

#ifdef __cplusplus
}
#endif

#endif
