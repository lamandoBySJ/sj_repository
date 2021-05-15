#include "platform/mbed_assert.h"
#include "platform/mbed_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

MBED_NORETURN void mbed_assert_internal(const char *expr, const char *file, int line)
{
    debug("mbed_assert_internal:file[%s],line[%d],expr:%s",file,line,expr);
   // while(1);
}
#ifdef __cplusplus
}
#endif