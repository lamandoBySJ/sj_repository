#ifndef GPIO_API_H
#define GPIO_API_H
#include "Arduino.h"
#include "hal/PinNames.h"
#include "platform/mbed.h"
#include <mutex>

typedef enum {
    IRQ_NONE,
    IRQ_RISE,
    IRQ_FALL
} gpio_irq_event;

//typedef struct gpio_irq_s gpio_irq_t;
typedef void (*gpio_irq_handler)(uint32_t id, gpio_irq_event event);

struct gpio_irq_control_t
{
        PinName pad;
        gpio_irq_handler handler;
        uint32_t id;
        gpio_irq_event events;
};
struct gpio_irq_t {
   // IRQn_Type irq_n;
    uint32_t irq_index;
    uint32_t event;
    PinName pin;
};

struct gpio_t
{
        PinName pin;
        PinDirection direction;
        PinMode mode;
        int value;
        void gpio_init_in_ex(gpio_t* gpio,PinName pin,PinMode mode);
        void gpio_init_in(gpio_t* gpio,PinName pin);
        void gpio_init_out(gpio_t* gpio,PinName pin);
        void gpio_init_out_ex(gpio_t* gpio,PinName pin,int value);
        void gpio_init_inout(gpio_t* gpio, PinName pin,PinDirection direction,PinMode mode,int value);

        void gpio_write(gpio_t* gpio,int value);
        int gpio_read(gpio_t* gpio);   
        int gpio_is_connected(gpio_t* gpio);
        void gpio_dir(gpio_t* gpio,PinDirection direction);
        void gpio_mode(gpio_t* gpio,PinMode mode);
        void write( int value);
        int read();
        int gpio_irq_init(gpio_irq_t *obj, PinName pin, gpio_irq_handler handler, uint32_t id);
        void gpio_irq_set(gpio_irq_t *obj, gpio_irq_event event, uint32_t enable);
private:
        rtos::Mutex _mtx;
        
};


#endif