#include "hal/ticker_api.h"

#ifdef __cplusplus
extern "C" {
#endif

void ticker_set_handler(const ticker_data_t *const ticker, ticker_event_handler handler){

}

void ticker_irq_handler(const ticker_data_t *const ticker){

}

void ticker_remove_event(const ticker_data_t *const ticker, ticker_event_t *obj){

}

void ticker_insert_event(const ticker_data_t *const ticker, ticker_event_t *obj, timestamp_t timestamp, uint32_t id){

}

void ticker_insert_event_us(const ticker_data_t *const ticker, ticker_event_t *obj, us_timestamp_t timestamp, uint32_t id){

}

timestamp_t ticker_read(const ticker_data_t *const ticker){
    return 0;
}


us_timestamp_t ticker_read_us(const ticker_data_t *const ticker){
    return 0;
}

int ticker_get_next_timestamp(const ticker_data_t *const ticker, timestamp_t *timestamp){
    return 0;
}

int ticker_get_next_timestamp_us(const ticker_data_t *const ticker, us_timestamp_t *timestamp){
    return 0;
}

void ticker_suspend(const ticker_data_t *const ticker){

}

void ticker_resume(const ticker_data_t *const ticker){

}


int _ticker_match_interval_passed(timestamp_t prev_tick, timestamp_t cur_tick, timestamp_t match_tick){
    return 0;
}


#ifdef __cplusplus
}
#endif