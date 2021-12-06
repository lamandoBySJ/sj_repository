#include "hal/gpio_api.h"
#include "hal/PinNames.h"

//static gpio_irq_control_t gpio_irq_control[40];

void gpio_t::gpio_init_in(gpio_t* gpio,PinName pin){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    gpio->pin=pin;
    gpio->direction=PinDirection::PIN_INPUT;
    gpio->mode=PinMode::Input;
    pinMode(gpio->pin,gpio->mode);
}
void gpio_t::gpio_init_in_ex(gpio_t* gpio,PinName pin,PinMode mode){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    gpio->pin=pin;
    gpio->direction=PinDirection::PIN_INPUT;
    gpio->mode=PinMode::InputPullup;
    pinMode(gpio->pin,gpio->mode);
    gpio->value=digitalRead(gpio->pin);
}
void gpio_t::gpio_init_out(gpio_t* gpio,PinName pin){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    gpio->pin=pin;
    gpio->direction=PinDirection::PIN_OUTPUT;
    gpio->mode=PinMode::Output;
    pinMode(gpio->pin,gpio->mode);
}
void gpio_t::gpio_init_out_ex(gpio_t* gpio,PinName pin,int value){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    gpio_init_out(gpio,pin);
    digitalWrite(gpio->pin,value);
}
void gpio_t::gpio_init_inout(gpio_t* gpio, PinName pin,PinDirection direction,PinMode mode,int value)
{std::lock_guard<rtos::Mutex> lck(_mtx);
    gpio->pin=pin;
    gpio->direction=direction;
    gpio->mode=mode;
    pinMode(pin,mode);
    if(direction==PinDirection::PIN_INPUT){
        gpio->value = digitalRead(pin);
    } else{
        digitalWrite(pin,value);
    }  
}
void gpio_t::gpio_write(gpio_t* gpio,int value)
{   
    std::lock_guard<rtos::Mutex> lck(_mtx);
    digitalWrite(gpio->pin,value);
}
int gpio_t::gpio_read(gpio_t* gpio){
    std::lock_guard<rtos::Mutex> lck(_mtx);
  return  digitalRead(gpio->pin);
}      
int gpio_t::gpio_is_connected(gpio_t* gpio){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    return  digitalRead(gpio->pin);
}
void gpio_t::gpio_dir(gpio_t* gpio,PinDirection direction)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    gpio->pin=pin;
    gpio->direction=direction;
    gpio->mode= direction==PinDirection::PIN_INPUT?PinMode::Input:PinMode::Output;
    pinMode(pin,mode);
    if(direction==PinDirection::PIN_INPUT){
        gpio->value = digitalRead(pin);
    } else{
        digitalWrite(pin,value);
    }  
}
void gpio_t::gpio_mode(gpio_t* gpio,PinMode mode)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    pinMode(gpio->pin,mode);
}
void gpio_t::write(int value)
{
    std::lock_guard<rtos::Mutex> lck(_mtx);
    digitalWrite(pin,value);
}
  int gpio_t::read()
  { 
      std::lock_guard<rtos::Mutex> lck(_mtx);
     return digitalRead(pin);
  }
  /*
  int gpio_t::gpio_irq_init(gpio_irq_t *obj, PinName pin, gpio_irq_handler handler, uint32_t id)
  {
    gpio_irq_control_t *control = &gpio_irq_control[pin];
    control->pad = pin;
    control->handler = handler;
    control->id = id;
    control->events = IRQ_NONE;
    return 0;
  }*/
  void gpio_t::gpio_irq_set(gpio_irq_t *obj, gpio_irq_event event, uint32_t enable)
  {
    obj->event=event;
    obj->irq_index=enable;
    obj->pin=pin;
  }