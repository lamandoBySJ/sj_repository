/* FreeRTOS includes. */
#include "rtos/tasks.h"
#include "freertos/list.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "rtos/stack_macros.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "platform/mbed_debug.h"

/* Values that can be assigned to the ucNotifyState member of the TCB. */
#define taskNOT_WAITING_NOTIFICATION	( ( uint8_t ) 0 )
#define taskWAITING_NOTIFICATION		( ( uint8_t ) 1 )
#define taskNOTIFICATION_RECEIVED		( ( uint8_t ) 2 )
#define portMUX_FREE_VAL		0xB33FFFFF

#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )
#define taskRECORD_READY_PRIORITY( uxPriority )	portRECORD_READY_PRIORITY( uxPriority, uxTopReadyPriority )

#define prvGetTCBFromHandle( pxHandle ) ( ( ( pxHandle ) == NULL ) ? pxCurrentTCB : ( pxHandle ) )

#define traceMOVED_TASK_TO_READY_STATE( pxTCB )
#define tracePOST_MOVED_TASK_TO_READY_STATE( pxTCB ) 

#if( configUSE_PREEMPTION == 0 )
	/* If the cooperative scheduler is being used then a yield should not be
	performed just because a higher priority task has been woken. */
	#define taskYIELD_IF_USING_PREEMPTION()
#else
	#define taskYIELD_IF_USING_PREEMPTION() portYIELD_WITHIN_API()
#endif

#define prvAddTaskToReadyList( pxTCB )																\
	traceMOVED_TASK_TO_READY_STATE( pxTCB );														\
	taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );												\
	vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ), &( ( pxTCB )->xStateListItem ) ); \
	tracePOST_MOVED_TASK_TO_READY_STATE( pxTCB )

static portMUX_TYPE mux = {.owner = portMUX_FREE_VAL,.count = 0,};

typedef tskTCB TCB_t;
/*lint -save -e956 A manual analysis and inspection has been used to determine
which static variables must be declared volatile. */
extern PRIVILEGED_DATA TCB_t *  pxCurrentTCB;
PRIVILEGED_DATA static volatile UBaseType_t uxSchedulerSuspended	= ( UBaseType_t ) pdFALSE;
/* Lists for ready and blocked tasks. --------------------
xDelayedTaskList1 and xDelayedTaskList2 could be move to function scople but
doing so breaks some kernel aware debuggers and debuggers that rely on removing
the static qualifier. */
PRIVILEGED_DATA static List_t pxReadyTasksLists[ configMAX_PRIORITIES ];/*< Prioritised ready tasks. */
//PRIVILEGED_DATA static List_t xDelayedTaskList1;						/*< Delayed tasks. */
//PRIVILEGED_DATA static List_t xDelayedTaskList2;						/*< Delayed tasks (two lists are used - one for delays that have overflowed the current tick count. */
PRIVILEGED_DATA static List_t * volatile pxDelayedTaskList;				/*< Points to the delayed task list currently being used. */
PRIVILEGED_DATA static List_t * volatile pxOverflowDelayedTaskList;		/*< Points to the delayed task list currently being used to hold tasks that have overflowed the current tick count. */
PRIVILEGED_DATA static List_t xPendingReadyList;	

#if( INCLUDE_vTaskDelete == 1 )
	//PRIVILEGED_DATA static List_t xTasksWaitingTermination;				/*< Tasks that have been deleted - but their memory not yet freed. */
	PRIVILEGED_DATA static volatile UBaseType_t uxDeletedTasksWaitingCleanUp = ( UBaseType_t ) 0U;

#endif

/* Global POSIX errno. Its value is changed upon context switching to match
the errno of the currently running task. */
#if ( configUSE_POSIX_ERRNO == 1 )
	int FreeRTOS_errno = 0;
#endif

/* Other file private variables. --------------------------------*/
PRIVILEGED_DATA static volatile UBaseType_t uxCurrentNumberOfTasks 	= ( UBaseType_t ) 0U;
PRIVILEGED_DATA static volatile TickType_t xTickCount 				= ( TickType_t ) configINITIAL_TICK_COUNT;
PRIVILEGED_DATA static volatile UBaseType_t uxTopReadyPriority 		= tskIDLE_PRIORITY;
PRIVILEGED_DATA static volatile BaseType_t xSchedulerRunning 		= pdFALSE;
PRIVILEGED_DATA static volatile TickType_t xPendedTicks 			= ( TickType_t ) 0U;
PRIVILEGED_DATA static volatile BaseType_t xYieldPending 			= pdFALSE;
PRIVILEGED_DATA static volatile BaseType_t xNumOfOverflows 			= ( BaseType_t ) 0;
//PRIVILEGED_DATA static UBaseType_t uxTaskNumber 					= ( UBaseType_t ) 0U;
PRIVILEGED_DATA static volatile TickType_t xNextTaskUnblockTime		= ( TickType_t ) 0U; /* Initialised to portMAX_DELAY before the scheduler starts. */
//PRIVILEGED_DATA static TaskHandle_t xIdleTaskHandle					= NULL;			/*< Holds the handle of the idle task.  The idle task is created automatically when the scheduler is started. */



//__attribute__((unused))  static void prvResetNextTaskUnblockTime( void );
#if ( configUSE_TICKLESS_IDLE ==1 )
static void prvResetNextTaskUnblockTime( void )
{
	TCB_t *pxTCB;

	if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
	{
		/* The new current delayed list is empty.  Set xNextTaskUnblockTime to
		the maximum possible value so it is	extremely unlikely that the
		if( xTickCount >= xNextTaskUnblockTime ) test will pass until
		there is an item in the delayed list. */
		xNextTaskUnblockTime = portMAX_DELAY;
	}
	else
	{
		/* The new current delayed list is not empty, get the value of
		the item at the head of the delayed list.  This is the time at
		which the task at the head of the delayed list should be removed
		from the Blocked state. */
		( pxTCB ) = listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList ); /*lint !e9079 void * is used as this macro is used with timers and co-routines too.  Alignment is known to be fine as the type of the pointer stored and retrieved is the same. */
		xNextTaskUnblockTime = listGET_LIST_ITEM_VALUE( &( ( pxTCB )->xStateListItem ) );
	}
}
#endif
struct tskTaskControlBlock; /* The old naming convention is used to prevent breaking kernel aware debuggers. */

#if( configUSE_TASK_NOTIFICATIONS == 1 )
	BaseType_t xTaskGenericNotify( TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction, uint32_t *pulPreviousNotificationValue )
	{
		TCB_t *pxTCB;
		BaseType_t xReturn = pdPASS;
		uint8_t ucOriginalNotifyState;
		configASSERT( xTaskToNotify );
		pxTCB = xTaskToNotify;
		portENTER_CRITICAL(&mux);
		{
			if( pulPreviousNotificationValue != NULL )
			{
				*pulPreviousNotificationValue = pxTCB->ulNotifiedValue;
			}
			ucOriginalNotifyState = pxTCB->ucNotifyState;
			pxTCB->ucNotifyState = taskNOTIFICATION_RECEIVED;
			switch( eAction )
			{
				case eSetBits	:
					pxTCB->ulNotifiedValue |= ulValue;
					break;

				case eIncrement	:
					( pxTCB->ulNotifiedValue )++;
					break;

				case eSetValueWithOverwrite	:
					pxTCB->ulNotifiedValue = ulValue;
					break;

				case eSetValueWithoutOverwrite :
					if( ucOriginalNotifyState != taskNOTIFICATION_RECEIVED )
					{
						pxTCB->ulNotifiedValue = ulValue;
					}
					else
					{
						/* The value could not be written to the task. */
						xReturn = pdFAIL;
					}
					break;

				case eNoAction:
					/* The task is being notified without its notify value being
					updated. */
					break;

				default:
					/* Should not get here if all enums are handled.
					Artificially force an assert by testing a value the
					compiler can't assume is const. */
					configASSERT( pxTCB->ulNotifiedValue == ~0UL );

					break;
			}
			traceTASK_NOTIFY();
			/* If the task is in the blocked state specifically to wait for a
			notification then unblock it now. */
			if( ucOriginalNotifyState == taskWAITING_NOTIFICATION )
			{
				( void ) uxListRemove( &( pxTCB->xStateListItem ) );
				prvAddTaskToReadyList( pxTCB );
			/*	traceMOVED_TASK_TO_READY_STATE( pxTCB );														
				taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );												
				vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ), &( ( pxTCB )->xStateListItem ) );
				tracePOST_MOVED_TASK_TO_READY_STATE( pxTCB );
				*/
				/* The task should not have been on an event list. */
				configASSERT( listLIST_ITEM_CONTAINER( &( pxTCB->xEventListItem ) ) == NULL );

				#if( configUSE_TICKLESS_IDLE != 0 )
				{
					/* If a task is blocked waiting for a notification then
					xNextTaskUnblockTime might be set to the blocked task's time
					out time.  If the task is unblocked for a reason other than
					a timeout xNextTaskUnblockTime is normally left unchanged,
					because it will automatically get reset to a new value when
					the tick count equals xNextTaskUnblockTime.  However if
					tickless idling is used it might be more important to enter
					sleep mode at the earliest possible time - so reset
					xNextTaskUnblockTime here to ensure it is updated at the
					earliest possible time. */
					prvResetNextTaskUnblockTime();
				}
				#endif

				if( pxTCB->uxPriority > pxCurrentTCB->uxPriority )
				{
					/* The notified task has a priority above the currently
					executing task so a yield is required. */
					taskYIELD_IF_USING_PREEMPTION();
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		portEXIT_CRITICAL(&mux);
		return xReturn;
	}
#endif

#if( configUSE_TASK_NOTIFICATIONS == 1 )

	BaseType_t xTaskGenericNotifyFromISR(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction, uint32_t *pulPreviousNotificationValue, BaseType_t *pxHigherPriorityTaskWoken )
	{
	TCB_t * pxTCB;
	uint8_t ucOriginalNotifyState;
	BaseType_t xReturn = pdPASS;
	UBaseType_t uxSavedInterruptStatus;

		configASSERT( xTaskToNotify );

		/* RTOS ports that support interrupt nesting have the concept of a
		maximum	system call (or maximum API call) interrupt priority.
		Interrupts that are	above the maximum system call priority are keep
		permanently enabled, even when the RTOS kernel is in a critical section,
		but cannot make any calls to FreeRTOS API functions.  If configASSERT()
		is defined in FreeRTOSConfig.h then
		portASSERT_IF_INTERRUPT_PRIORITY_INVALID() will result in an assertion
		failure if a FreeRTOS API function is called from an interrupt that has
		been assigned a priority above the configured maximum system call
		priority.  Only FreeRTOS functions that end in FromISR can be called
		from interrupts	that have been assigned a priority at or (logically)
		below the maximum system call interrupt priority.  FreeRTOS maintains a
		separate interrupt safe API to ensure interrupt entry is as fast and as
		simple as possible.  More information (albeit Cortex-M specific) is
		provided on the following link:
		http://www.freertos.org/RTOS-Cortex-M3-M4.html */
		portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

		pxTCB = xTaskToNotify;

		uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
		{
			if( pulPreviousNotificationValue != NULL )
			{
				*pulPreviousNotificationValue = pxTCB->ulNotifiedValue;
			}

			ucOriginalNotifyState = pxTCB->ucNotifyState;
			pxTCB->ucNotifyState = taskNOTIFICATION_RECEIVED;

			switch( eAction )
			{
				case eSetBits	:
					pxTCB->ulNotifiedValue |= ulValue;
					break;

				case eIncrement	:
					( pxTCB->ulNotifiedValue )++;
					break;

				case eSetValueWithOverwrite	:
					pxTCB->ulNotifiedValue = ulValue;
					break;

				case eSetValueWithoutOverwrite :
					if( ucOriginalNotifyState != taskNOTIFICATION_RECEIVED )
					{
						pxTCB->ulNotifiedValue = ulValue;
					}
					else
					{
						/* The value could not be written to the task. */
						xReturn = pdFAIL;
					}
					break;

				case eNoAction :
					/* The task is being notified without its notify value being
					updated. */
					break;

				default:
					/* Should not get here if all enums are handled.
					Artificially force an assert by testing a value the
					compiler can't assume is const. */
					configASSERT( pxTCB->ulNotifiedValue == ~0UL );
					break;
			}

			traceTASK_NOTIFY_FROM_ISR();

			/* If the task is in the blocked state specifically to wait for a
			notification then unblock it now. */
			if( ucOriginalNotifyState == taskWAITING_NOTIFICATION )
			{
				/* The task should not have been on an event list. */
				configASSERT( listLIST_ITEM_CONTAINER( &( pxTCB->xEventListItem ) ) == NULL );

				if( uxSchedulerSuspended == ( UBaseType_t ) pdFALSE )
				{
					( void ) uxListRemove( &( pxTCB->xStateListItem ) );
					prvAddTaskToReadyList( pxTCB );
				}
				else
				{
					/* The delayed and ready lists cannot be accessed, so hold
					this task pending until the scheduler is resumed. */
					vListInsertEnd( &( xPendingReadyList ), &( pxTCB->xEventListItem ) );
				}

				if( pxTCB->uxPriority > pxCurrentTCB->uxPriority )
				{
					/* The notified task has a priority above the currently
					executing task so a yield is required. */
					if( pxHigherPriorityTaskWoken != NULL )
					{
						*pxHigherPriorityTaskWoken = pdTRUE;
					}

					/* Mark that a yield is pending in case the user is not
					using the "xHigherPriorityTaskWoken" parameter to an ISR
					safe FreeRTOS function. */
					xYieldPending = pdTRUE;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
		}
		portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

		return xReturn;
	}

#endif

char *pcTaskGetName( TaskHandle_t xTaskToQuery ) /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
{
TCB_t *pxTCB;

	/* If null is passed in here then the name of the calling task is being
	queried. */
	pxTCB = prvGetTCBFromHandle( xTaskToQuery );
	configASSERT( pxTCB );
	return &( pxTCB->pcTaskName[ 0 ] );
}
