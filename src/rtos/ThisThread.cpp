#include "rtos/ThisThread.h"
#include "Arduino.h"
#include <chrono>
#include <type_traits>
using namespace rtos;
using namespace mstd;



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
    //ThisThread::sleep_for(Kernel::Clock::duration_u32(1000));
   // ThisThread::sleep_for(std::chrono::seconds (1));
 
}