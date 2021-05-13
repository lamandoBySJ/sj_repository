#ifndef __INC_TASK_H
#define __INC_TASK_H

#include "Arduino.h"
#include "rtos/list.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#ifndef portALLOCATE_SECURE_CONTEXT
#define portALLOCATE_SECURE_CONTEXT( ulSecureStackSize )
#endif

#ifndef configSTACK_DEPTH_TYPE
#define configSTACK_DEPTH_TYPE uint16_t
#endif
#ifndef portDONT_DISCARD
#define portDONT_DISCARD
#endif
#ifndef configINITIAL_TICK_COUNT
#define configINITIAL_TICK_COUNT 0
#endif

#ifndef traceTASK_NOTIFY
	#define traceTASK_NOTIFY()
#endif
#ifndef traceTASK_NOTIFY_FROM_ISR
	#define traceTASK_NOTIFY_FROM_ISR()
#endif



BaseType_t xTaskGenericNotify( TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction, uint32_t *pulPreviousNotificationValue ) PRIVILEGED_FUNCTION;
#define xTaskNotify( xTaskToNotify, ulValue, eAction ) xTaskGenericNotify( ( xTaskToNotify ), ( ulValue ), ( eAction ), NULL )
#define xTaskNotifyAndQuery( xTaskToNotify, ulValue, eAction, pulPreviousNotifyValue ) xTaskGenericNotify( ( xTaskToNotify ), ( ulValue ), ( eAction ), ( pulPreviousNotifyValue ) )


#endif