#ifndef TIMEOUT_CHECKER_H
#define TIMEOUT_CHECKER_H
#include "platform_debug.h"

class TimeoutChecker
{
public:
    TimeoutChecker():_thread(),_timeout(60),_decided(false)
    {

    }
    TimeoutChecker(uint32_t timeout):_thread(),_timeout(timeout),_decided(false)
    {

    }
    void startup();
    void task_restart_countdown();
    bool makesDecision(uint32_t flags=0);
private:
    rtos::Thread _thread;
    uint32_t _timeout;
    bool _decided;
};
#endif