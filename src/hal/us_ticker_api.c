#include "hal/us_ticker_api.h"
//#include "cyhal_hw_resources.h"
#include "Arduino.h"
#include "esp_timer.h"
#define CY_IP_MXTCPWM 1
#define CY_US_TICKER_IRQ_PRIORITY 3

#ifdef __cplusplus
extern "C" {
#endif
/*
typedef struct{

}cyhal_timer_event_t;
typedef struct{

}cyhal_timer_t;

typedef enum{
    CY_SYSPM_SUCCESS=0
}cy_en_syspm_status_t;

typedef struct{

}cy_stc_syspm_callback_params_t;
typedef struct{

}cy_en_syspm_callback_mode_t;
typedef struct{

}cy_stc_syspm_callback_t;


static cyhal_timer_t cy_us_timer;
*/
static bool cy_us_ticker_initialized = false;
static ticker_info_t cy_us_ticker_info;
static esp_timer_handle_t esp_timer=NULL;
/*
static cy_en_syspm_status_t cy_us_ticker_pm_callback(cy_stc_syspm_callback_params_t *params, cy_en_syspm_callback_mode_t mode)
{
   // if (mode == CY_SYSPM_AFTER_TRANSITION) {
   //     Cy_TCPWM_Counter_Enable(cy_us_timer.tcpwm.base, cy_us_timer.tcpwm.resource.channel_num);
   //     Cy_TCPWM_TriggerStart(cy_us_timer.tcpwm.base, 1u << cy_us_timer.tcpwm.resource.channel_num);
   // }
    return CY_SYSPM_SUCCESS;
}

static cy_stc_syspm_callback_params_t cy_us_ticker_pm_params;
static cy_stc_syspm_callback_t  cy_us_ticker_pm_data = {
   // .callback = &cy_us_ticker_pm_callback,
   // .type = CY_SYSPM_DEEPSLEEP,
   // .callbackParams = &cy_us_ticker_pm_params,
};

static void cy_us_ticker_irq_handler(MBED_UNUSED void *arg, MBED_UNUSED cyhal_timer_event_t event)
{
    us_ticker_irq_handler();
}
*/
//static cyhal_timer_t cy_us_timer;
void cy_us_ticker_start()
{
    //cyhal_timer_start(&cy_us_timer);
}

void cy_us_ticker_stop()
{
   // cyhal_timer_stop(&cy_us_timer);
}
static void IRAM_ATTR us_ticker_callback(void* arg)
{
    us_ticker_irq_handler();
}
//static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
void us_ticker_init(void)
{
    if (!cy_us_ticker_initialized) {
            /* There are two timers, Timer0 and Timer1, available on the PSoC64.
            * Timer0 has 8 channels and Timer1 has 24 channels. TF-M regression
            * tests make use of Timer0 Channel 1 and Timer0 Channel 2. Therefore,
            * reserve the timer channels used by TF-M. This approach can be
            * replaced once we have a way to allocate dedicated timers for TF-M
            * and Mbed OS. */
            //uint32_t div_value = 80;
          //  cy_us_ticker_info.frequency =  1;
           // cy_us_ticker_info.bits = 32;
            uint32_t milliseconds=1000;
            static esp_timer_create_args_t _timerConfig;
            _timerConfig.arg = NULL;
            _timerConfig.callback = us_ticker_callback;
            _timerConfig.dispatch_method = ESP_TIMER_TASK;
            _timerConfig.name = "Ticker";
            if (esp_timer) {
                esp_timer_stop(esp_timer);
                esp_timer_delete(esp_timer);
            }
            esp_err_t err =  esp_timer_create(&_timerConfig, &esp_timer);
            err = esp_timer_start_periodic(esp_timer, milliseconds * 1000ULL);
            if(err == ESP_OK)
            {
                debug("timer1 create ok!\r\n");        
            }

            cy_us_ticker_initialized = true;
    }
    us_ticker_disable_interrupt();
}
void us_ticker_free(void)
{

}
uint32_t us_ticker_read(void)
{
    return (uint32_t)esp_timer_get_time();
}
void us_ticker_set_interrupt(timestamp_t timestamp)
{

}
void us_ticker_disable_interrupt(void)
{

}
void us_ticker_clear_interrupt(void)
{

}
void us_ticker_fire_interrupt(void)
{

}
const ticker_info_t *us_ticker_get_info(void)
{
    return &cy_us_ticker_info;
}

#ifdef __cplusplus
}
#endif



/** @}*/
