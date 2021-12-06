
#pragma once

#include "MCP23017.h"
#include "platform_debug.h"
#include "app/IOExpander/MCPBase.h"

class MCP23017Builder  : public MCPBase, public MCP23017
{
public:
    explicit MCP23017Builder()=delete;
    MCP23017Builder(rtos::Mutex &mtx,TwoWire& wire,uint8_t rst=-1):MCPBase(mtx,wire),MCP23017(0x27,wire),_mtx(mtx),_rst(rst)
    
    {

    }
    explicit MCP23017Builder(const MCP23017Builder& builder):MCP23017Builder(builder._mtx,builder._wire,builder._rst)
    {

    }
    explicit MCP23017Builder(MCP23017Builder&& builder):MCP23017Builder(builder._mtx,builder._wire,builder._rst)
    {

    }
    MCP23017Builder& operator=(const MCP23017Builder&)=delete;
    MCP23017Builder& operator=(MCP23017Builder&&)=delete;
    
    
    ~MCP23017Builder()=default;
    
    void init(uint8_t modePortA=0b11111111,uint8_t modePortB=0b11111111) override
    {
        MCP23017::init();
        MCP23017::portMode(MCP23017Port::A,  modePortA);          //Port A as output 0
        MCP23017::portMode(MCP23017Port::B,  modePortB);         //Port B as input 0b11111111
        //mcp.portMode(MCP23017Port::B, 0); //Port B as output
        MCP23017::writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
        MCP23017::writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B
    }
   
    void power_on() override;
    void writePort(MCP23017Port port,uint8_t value) override
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
         MCP23017::writePort(port, value);
    }
    uint8_t readPort(MCP23017Port port) override
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        return  MCP23017::readPort(port);
    }
    void digitalWrite(uint8_t pin, uint8_t state) override
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        MCP23017::digitalWrite(pin,state);
    }

    uint8_t digitalRead(uint8_t pin) override
    {
        std::lock_guard<rtos::Mutex> lck(_mtx);
        return  MCP23017::digitalRead(pin);
    }
private:
    rtos::Mutex& _mtx;
    int8_t _rst;
    
};