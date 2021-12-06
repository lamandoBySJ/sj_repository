#pragma once
#include "Arduino.h"
#include "MCP23017.h"
#include "platform/mbed_critical.h"
#include "I2CBus.h"

enum class PinName : uint8_t
{
    GPA0=0,GPA1,GPA2,GPA3,GPA4,GPA5,GPA6,GPA7,
    GPB0=8,GPB1,GPB2,GPB3,GPB4,GPB5,GPB6,GPB7
};



class MCPBase : public I2CBus
{
public:
    MCPBase(rtos::Mutex& mtx,TwoWire& wire):I2CBus(mtx,wire)
    {
       // Serial.println("--- MCPBase ---");
    }
    ~MCPBase()=default;
    virtual void power_on() = 0;
    virtual void init(uint8_t modePortA=0b11111111,uint8_t modePortB=0b11111111) = 0;
    virtual void writePort(MCP23017Port port,uint8_t value) = 0;
    virtual uint8_t readPort(MCP23017Port port) = 0;
    virtual void digitalWrite(uint8_t pin, uint8_t state)= 0;
    virtual uint8_t digitalRead(uint8_t pin)= 0;

    static uint8_t& port_a_mask(){
        static uint8_t _mask=0xFF;
        return _mask;
    }
    static uint8_t& port_b_mask(){
        static uint8_t _mask=0xFF;
        return _mask;
    }
    static	MCPBase** getBuilder(){
		static MCPBase* _builder;
		return &_builder;
	}
};

class DigitalIn //: public MCPBase
{
public:
    DigitalIn():_pin(PinName::GPA0),_value(0){};
    explicit  DigitalIn(PinName pin):_pin(pin),_value(0){}
    DigitalIn(PinName pin,uint8_t value):_pin(pin),_value(value){}
    ~DigitalIn()=default;
    explicit  operator uint8_t(){
        return DigitalIn::read(_pin);
    }
    uint8_t read(){
       return _value=DigitalIn::read(_pin);
    }
    static uint8_t read(PinName pin){ 
        return (*MCPBase::getBuilder())->digitalRead(static_cast<uint8_t>(pin));
    }
private:
    PinName _pin;
    uint8_t _value;
};
class DigitalOut //: public MCPBase
{
public:
    DigitalOut():_pin(PinName::GPA0),_value(0){};
    DigitalOut(PinName pin):_pin(pin),_value(0){}
    DigitalOut(PinName pin,uint8_t value):_pin(pin),_value(value){}
    ~DigitalOut()=default;
    void write(uint8_t value){
        DigitalOut::write(_pin,_value=value);
    }
    DigitalOut & operator = (uint8_t value){
        DigitalOut::write(_pin,_value=value);
        return *this;
    }
    DigitalOut & operator = (DigitalOut &rhs){
       DigitalOut::write(_pin,_value=rhs.read());
        return *this;
    }
    uint8_t read(){
       return _value;
    }
    static void write(PinName pin,uint8_t value){
        (*MCPBase::getBuilder())->digitalWrite(static_cast<uint8_t>(pin),value);
    }
private:
PinName _pin;
uint8_t _value;
};
class PortIn // public MCPBase
{
public:
    PortIn()=delete;
    PortIn (MCP23017Port port,uint8_t mask=0xFF):_port(port){}
    ~PortIn()=delete;
    static uint8_t read(MCP23017Port port)
    {
       return (*MCPBase::getBuilder())->readPort(port);
    }
    uint8_t read()
    {
        return read(_port);
    }
    operator uint8_t()
    {
        return read(_port);
    }
private:
    MCP23017Port _port;

};
class PortOut //: public MCPBase
{
public:
    PortOut()=default;
    PortOut(MCP23017Port port,uint8_t mask = 0xFF):_port(port){}
    ~PortOut()=default;
    static void write(MCP23017Port port,uint8_t value){
        (*MCPBase::getBuilder())->writePort(port,value);
    }
    void write(uint8_t value)
    {
        write(_port,value);
    }
    uint8_t read()
    {
        return (*MCPBase::getBuilder())->readPort(_port);
    }
    PortOut &operator= (int value)
    {
        write(value);
        return *this;
    }
    PortOut &operator = (PortOut &rhs)
    {
        write(rhs.read());
        return *this;
    }
    operator uint8_t()
    {
        return read();
    }
private:
MCP23017Port _port;

};

