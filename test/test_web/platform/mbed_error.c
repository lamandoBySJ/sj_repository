/* mbed Microcontroller Library
 * Copyright (c) 2006-2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Arduino.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "platform/mbed_atomic.h"
#include "platform/mbed_critical.h"
#include "platform/mbed_error.h"

#ifdef MBED_CONF_RTOS_PRESENT
#include "rtx_os.h"
#endif

#if DEVICE_STDIO_MESSAGES
#include <stdio.h>
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef NDEBUG
#define ERROR_REPORT(ctx, error_msg, error_filename, error_line) print_error_report(ctx, error_msg, error_filename, error_line)
static void print_error_report(const mbed_error_ctx *ctx, const char *, const char *error_filename, int error_line);
#else
#define ERROR_REPORT(ctx, error_msg, error_filename, error_line) ((void) 0)
#endif

bool mbed_error_in_progress;
static core_util_atomic_flag halt_in_progress = CORE_UTIL_ATOMIC_FLAG_INIT;
static int error_count = 0;
static mbed_error_ctx first_error_ctx = {0};

static mbed_error_ctx last_error_ctx = {0};
static mbed_error_hook_t error_hook = NULL;
static mbed_error_status_t handle_error(mbed_error_status_t error_status, unsigned int error_value, const char *filename, int line_number, void *caller);

#if MBED_CONF_PLATFORM_CRASH_CAPTURE_ENABLED
#define report_error_ctx MBED_CRASH_DATA.error.context
static bool is_reboot_error_valid = false;
#endif

//Helper function to halt the system
static MBED_NORETURN void mbed_halt_system(void)
{
   
}

WEAK MBED_NORETURN void error(const char *format, ...)
{
    // Prevent recursion if error is called again during store+print attempt
    if (!core_util_atomic_exchange_bool(&mbed_error_in_progress, true)) {
        handle_error(MBED_ERROR_UNKNOWN, 0, NULL, 0, MBED_CALLER_ADDR());
        ERROR_REPORT(&last_error_ctx, "Fatal Run-time error", NULL, 0);

#ifndef NDEBUG
        va_list arg;
        va_start(arg, format);
        mbed_error_vprintf(format, arg);
        va_end(arg);
        // Add a newline to prevent any line buffering
        mbed_error_puts("\n");
#endif
    }

    mbed_halt_system();
}

static inline bool mbed_error_is_hw_fault(mbed_error_status_t error_status)
{
    return (error_status == MBED_ERROR_MEMMANAGE_EXCEPTION ||
            error_status == MBED_ERROR_BUSFAULT_EXCEPTION ||
            error_status == MBED_ERROR_USAGEFAULT_EXCEPTION ||
            error_status == MBED_ERROR_HARDFAULT_EXCEPTION);
}

static bool mbed_error_is_handler(const mbed_error_ctx *ctx)
{
    return false;
}

//Set an error status with the error handling system
static mbed_error_status_t handle_error(mbed_error_status_t error_status, unsigned int error_value, const char *filename, int line_number, void *caller)
{
    return MBED_SUCCESS;
}

WEAK void mbed_error_hook(const mbed_error_ctx *error_context)
{
    //Dont do anything here, let application override this if required.
}

WEAK void mbed_error_reboot_callback(mbed_error_ctx *error_context)
{
    //Dont do anything here, let application override this if required.
}

//Initialize Error handling system and report any errors detected on rebooted
mbed_error_status_t mbed_error_initialize(void)
{
    return MBED_SUCCESS;
}

//Return the first error
mbed_error_status_t mbed_get_first_error(void)
{
    //return the first error recorded
    return first_error_ctx.error_status;
}

//Return the last error
mbed_error_status_t mbed_get_last_error(void)
{
    //return the last error recorded
    return last_error_ctx.error_status;
}

//Gets the current error count
int mbed_get_error_count(void)
{
    //return the current error count
    return error_count;
}

//Reads the fatal error occurred" flag
bool mbed_get_error_in_progress(void)
{
    return core_util_atomic_load_bool(&mbed_error_in_progress);
}

//Sets a non-fatal error
mbed_error_status_t mbed_warning(mbed_error_status_t error_status, const char *error_msg, unsigned int error_value, const char *filename, int line_number)
{
    return handle_error(error_status, error_value, filename, line_number, MBED_CALLER_ADDR());
}

//Sets a fatal error, this function is marked WEAK to be able to override this for some tests
WEAK MBED_NORETURN mbed_error_status_t mbed_error(mbed_error_status_t error_status, const char *error_msg, unsigned int error_value, const char *filename, int line_number)
{
    while(1);
}

//Register an application defined callback with error handling
MBED_DEPRECATED("Use an overridden mbed_error_hook() function instead")
mbed_error_status_t mbed_set_error_hook(mbed_error_hook_t error_hook_in)
{
    //register the new hook/callback
    if (error_hook_in != NULL) {
        error_hook = error_hook_in;
        return MBED_SUCCESS;
    }

    return MBED_ERROR_INVALID_ARGUMENT;
}

//Reset the reboot error context
mbed_error_status_t mbed_reset_reboot_error_info()
{

    return MBED_SUCCESS;
}

//Reset the reboot error context
mbed_error_status_t mbed_reset_reboot_count()
{
#if MBED_CONF_PLATFORM_CRASH_CAPTURE_ENABLED
    if (is_reboot_error_valid) {
        uint32_t crc_val = 0;
        core_util_critical_section_enter();
        report_error_ctx.error_reboot_count = 0;//Set reboot count to 0
        //Update CRC
        crc_val = mbed_tiny_compute_crc32(&report_error_ctx, offsetof(mbed_error_ctx, crc_error_ctx));
        report_error_ctx.crc_error_ctx = crc_val;
        core_util_critical_section_exit();
        return MBED_SUCCESS;
    }
#endif
    return MBED_ERROR_ITEM_NOT_FOUND;
}

//Retrieve the reboot error context
mbed_error_status_t mbed_get_reboot_error_info(mbed_error_ctx *error_info)
{
    mbed_error_status_t status = MBED_ERROR_ITEM_NOT_FOUND;
#if MBED_CONF_PLATFORM_CRASH_CAPTURE_ENABLED
    if (is_reboot_error_valid) {
        if (error_info != NULL) {
            *error_info = report_error_ctx;
            status = MBED_SUCCESS;
        } else {
            status = MBED_ERROR_INVALID_ARGUMENT;
        }
    }
#endif
    return status;
}

//Retrieve the first error context from error log
mbed_error_status_t mbed_get_first_error_info(mbed_error_ctx *error_info)
{
    *error_info = first_error_ctx;
    return MBED_SUCCESS;
}

//Retrieve the last error context from error log
mbed_error_status_t mbed_get_last_error_info(mbed_error_ctx *error_info)
{
    *error_info = last_error_ctx;
    return MBED_SUCCESS;
}

//Makes an mbed_error_status_t value
mbed_error_status_t mbed_make_error(mbed_error_type_t error_type, mbed_module_type_t entity, mbed_error_code_t error_code)
{
    switch (error_type) {
        case MBED_ERROR_TYPE_POSIX:
            if (error_code >= MBED_POSIX_ERROR_BASE && error_code <= MBED_SYSTEM_ERROR_BASE) {
                return -error_code;
            }
            break;

        case MBED_ERROR_TYPE_SYSTEM:
            if (error_code >= MBED_SYSTEM_ERROR_BASE && error_code <= MBED_CUSTOM_ERROR_BASE) {
                return MAKE_MBED_ERROR(MBED_ERROR_TYPE_SYSTEM, entity, error_code);
            }
            break;

        case MBED_ERROR_TYPE_CUSTOM:
            if (error_code >= MBED_CUSTOM_ERROR_BASE) {
                return MAKE_MBED_ERROR(MBED_ERROR_TYPE_CUSTOM, entity, error_code);
            }
            break;

        default:
            break;
    }

    //If we are passed incorrect values return a generic system error
    return MAKE_MBED_ERROR(MBED_ERROR_TYPE_SYSTEM, MBED_MODULE_UNKNOWN, MBED_ERROR_CODE_UNKNOWN);
}

/**
 * Clears all the last error, error count and all entries in the error log.
 * @return                      0 or MBED_SUCCESS on success.
 *
 */
mbed_error_status_t mbed_clear_all_errors(void)
{
    mbed_error_status_t status = MBED_SUCCESS;

    //Make sure we dont multiple clients resetting
    core_util_critical_section_enter();
    //Clear the error and context capturing buffer
    memset(&last_error_ctx, 0, sizeof(mbed_error_ctx));
    //reset error count to 0
    error_count = 0;
#if MBED_CONF_PLATFORM_ERROR_HIST_ENABLED
    status = mbed_error_hist_reset();
#endif
    core_util_critical_section_exit();

    return status;
}

#ifdef MBED_CONF_RTOS_PRESENT
static inline const char *name_or_unnamed(const osRtxThread_t *thread)
{
    const char *unnamed = "<unnamed>";

    if (!thread) {
        return unnamed;
    }

    const char *name = thread->name;
    return name ? name : unnamed;
}
#endif // MBED_CONF_RTOS_PRESENT

#if MBED_STACK_DUMP_ENABLED
/** Prints stack dump from given stack information.
 * The arguments should be given in address raw value to check alignment.
 * @param stack_start The address of stack start.
 * @param stack_size The size of stack
 * @param stack_sp The stack pointer currently at. */
static void print_stack_dump_core(uint32_t stack_start, uint32_t stack_size, uint32_t stack_sp, const char *postfix)
{

}

static void print_stack_dump(uint32_t stack_start, uint32_t stack_size, uint32_t stack_sp, const mbed_error_ctx *ctx)
{
   
}
#endif  // MBED_STACK_DUMP_ENABLED

#if MBED_CONF_PLATFORM_ERROR_ALL_THREADS_INFO && defined(MBED_CONF_RTOS_PRESENT)
/* Prints info of a thread(using osRtxThread_t struct)*/
static void print_thread(const osRtxThread_t *thread)
{
   
}

/* Prints thread info from a list */
static void print_threads_info(const osRtxThread_t *threads)
{
    while (threads != NULL) {
        print_thread(threads);
        threads = threads->thread_next;
    }
}
#endif

#ifndef NDEBUG
#define GET_TARGET_NAME_STR(tgt_name)   #tgt_name
#define GET_TARGET_NAME(tgt_name)       GET_TARGET_NAME_STR(tgt_name)
static void print_error_report(const mbed_error_ctx *ctx, const char *error_msg, const char *error_filename, int error_line)
{
    int error_code = MBED_GET_ERROR_CODE(ctx->error_status);
    int error_module = MBED_GET_ERROR_MODULE(ctx->error_status);

    mbed_error_printf("\n\n++ MbedOS Error Info ++\nError Status: 0x%X Code: %d Module: %d\nError Message: ", ctx->error_status, error_code, error_module);

    switch (error_code) {
        //These are errors reported by kernel handled from mbed_rtx_handlers
        case MBED_ERROR_CODE_RTOS_EVENT:
            mbed_error_printf("Kernel Error: 0x%" PRIX32 ", ", ctx->error_value);
            break;

        case MBED_ERROR_CODE_RTOS_THREAD_EVENT:
            mbed_error_printf("Thread: 0x%" PRIX32 ", ", ctx->error_value);
            break;

        case MBED_ERROR_CODE_RTOS_MUTEX_EVENT:
            mbed_error_printf("Mutex: 0x%" PRIX32 ", ", ctx->error_value);
            break;

        case MBED_ERROR_CODE_RTOS_SEMAPHORE_EVENT:
            mbed_error_printf("Semaphore: 0x%" PRIX32 ", ", ctx->error_value);
            break;

        case MBED_ERROR_CODE_RTOS_MEMORY_POOL_EVENT:
            mbed_error_printf("MemoryPool: 0x%" PRIX32 ", ", ctx->error_value);
            break;

        case MBED_ERROR_CODE_RTOS_EVENT_FLAGS_EVENT:
            mbed_error_printf("EventFlags: 0x%" PRIX32 ", ", ctx->error_value);
            break;

        case MBED_ERROR_CODE_RTOS_TIMER_EVENT:
            mbed_error_printf("Timer: 0x%" PRIX32 ", ", ctx->error_value);
            break;

        case MBED_ERROR_CODE_RTOS_MESSAGE_QUEUE_EVENT:
            mbed_error_printf("MessageQueue: 0x%" PRIX32 ", ", ctx->error_value);
            break;

        case MBED_ERROR_CODE_ASSERTION_FAILED:
            mbed_error_printf("Assertion failed: ");
            break;

        default:
            //Nothing to do here, just print the error info down
            break;
    }
    mbed_error_puts(error_msg);
    mbed_error_printf("\nLocation: 0x%" PRIX32, ctx->error_address);

    /* We print the filename passed in, not any filename in the context. This
     * avoids the console print for mbed_error being limited to the presence
     * and length of the filename storage. Note that although the MBED_ERROR
     * macro compiles out filenames unless platform.error-filename-capture-enabled
     * is turned on, MBED_ASSERT always passes filenames, and other direct
     * users of mbed_error() may also choose to.
     */
    if (error_filename) {
        mbed_error_puts("\nFile: ");
        mbed_error_puts(error_filename);
        mbed_error_printf("+%d", error_line);
    }

    mbed_error_printf("\nError Value: 0x%" PRIX32, ctx->error_value);
#ifdef MBED_CONF_RTOS_PRESENT
    bool is_handler = mbed_error_is_handler(ctx);
    mbed_error_printf("\nCurrent Thread: %s%s Id: 0x%" PRIX32 " Entry: 0x%" PRIX32 " StackSize: 0x%" PRIX32 " StackMem: 0x%" PRIX32 " SP: 0x%" PRIX32 " ",
                      name_or_unnamed((osRtxThread_t *)ctx->thread_id), is_handler ? " <handler>" : "",
                      ctx->thread_id, ctx->thread_entry_address, ctx->thread_stack_size, ctx->thread_stack_mem, ctx->thread_current_sp);
#endif

#if MBED_STACK_DUMP_ENABLED
    print_stack_dump(ctx->thread_stack_mem, ctx->thread_stack_size, ctx->thread_current_sp, ctx);
#endif

#if MBED_CONF_PLATFORM_ERROR_ALL_THREADS_INFO && defined(MBED_CONF_RTOS_PRESENT)
    mbed_error_printf("\nNext:");
    print_thread(osRtxInfo.thread.run.next);

    mbed_error_printf("\nReady:");
    print_threads_info(osRtxInfo.thread.ready.thread_list);

    mbed_error_printf("\nWait:");
    print_threads_info(osRtxInfo.thread.wait_list);

    mbed_error_printf("\nDelay:");
    print_threads_info(osRtxInfo.thread.delay_list);
#endif
#if !defined(MBED_SYS_STATS_ENABLED)
    mbed_error_printf("\nFor more info, visit: https://mbed.com/s/error?error=0x%08X&tgt=" GET_TARGET_NAME(TARGET_NAME), ctx->error_status);
#else
    mbed_stats_sys_t sys_stats;
    mbed_stats_sys_get(&sys_stats);
    mbed_error_printf("\nFor more info, visit: https://mbed.com/s/error?error=0x%08X&osver=%" PRId32 "&core=0x%08" PRIX32 "&comp=%d&ver=%" PRIu32 "&tgt=" GET_TARGET_NAME(TARGET_NAME), ctx->error_status, sys_stats.os_version, sys_stats.cpu_id, sys_stats.compiler_id, sys_stats.compiler_version);
#endif

    mbed_error_printf("\n-- MbedOS Error Info --\n");
}
#endif //ifndef NDEBUG


#if MBED_CONF_PLATFORM_ERROR_HIST_ENABLED
//Retrieve the error context from error log at the specified index
mbed_error_status_t mbed_get_error_hist_info(int index, mbed_error_ctx *error_info)
{
    return mbed_error_hist_get(index, error_info);
}

//Retrieve the error log count
int mbed_get_error_hist_count(void)
{
    return mbed_error_hist_get_count();
}

mbed_error_status_t mbed_save_error_hist(const char *path)
{
    mbed_error_status_t ret = MBED_SUCCESS;
    mbed_error_ctx ctx = {0};
    int log_count = mbed_error_hist_get_count();
    FILE *error_log_file = NULL;

    //Ensure path is valid
    if (path == NULL) {
        ret = MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_INVALID_ARGUMENT);
        goto exit;
    }

    //Open the file for saving the error log info
    if ((error_log_file = fopen(path, "w")) == NULL) {
        ret = MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_OPEN_FAILED);
        goto exit;
    }

    //First store the first and last errors
    if (fprintf(error_log_file, "\nFirst Error: Status:0x%x ThreadId:0x%x Address:0x%x Value:0x%x\n",
                (unsigned int)first_error_ctx.error_status,
                (unsigned int)first_error_ctx.thread_id,
                (unsigned int)first_error_ctx.error_address,
                (unsigned int)first_error_ctx.error_value) <= 0) {
        ret = MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_WRITE_FAILED);
        goto exit;
    }

    if (fprintf(error_log_file, "\nLast Error: Status:0x%x ThreadId:0x%x Address:0x%x Value:0x%x\n",
                (unsigned int)last_error_ctx.error_status,
                (unsigned int)last_error_ctx.thread_id,
                (unsigned int)last_error_ctx.error_address,
                (unsigned int)last_error_ctx.error_value) <= 0) {
        ret = MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_WRITE_FAILED);
        goto exit;
    }

    //Update with error log info
    while (--log_count >= 0) {
        mbed_error_hist_get(log_count, &ctx);
        //first line of file will be error log count
        if (fprintf(error_log_file, "\n%d: Status:0x%x ThreadId:0x%x Address:0x%x Value:0x%x\n",
                    log_count,
                    (unsigned int)ctx.error_status,
                    (unsigned int)ctx.thread_id,
                    (unsigned int)ctx.error_address,
                    (unsigned int)ctx.error_value) <= 0) {
            ret = MBED_MAKE_ERROR(MBED_MODULE_PLATFORM, MBED_ERROR_CODE_WRITE_FAILED);
            goto exit;
        }
    }

exit:
    fclose(error_log_file);

    return ret;
}
#endif
