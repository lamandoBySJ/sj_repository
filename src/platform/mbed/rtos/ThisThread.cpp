#include "rtos/ThisThread.h"
#include "Arduino.h"
using namespace rtos;

void ThisThread::sleep_for(uint32_t millisec)
{
    TickType_t xDelay = millisec / portTICK_PERIOD_MS;
	vTaskDelay(xDelay);
}

void ThisThread::sleep_for(Kernel::Clock::duration_u32 rel_time)
{
    TickType_t xDelay = rel_time.count()/ portTICK_PERIOD_MS;
	vTaskDelay(xDelay);
    //sleep_for(rel_time.count());
}