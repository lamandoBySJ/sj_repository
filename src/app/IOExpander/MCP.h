#pragma once

#include "app/IOExpander/MCPBase.h"
#include "app/IOExpander/MCPDirector.h"

class MCP   :  public DigitalIn
            ,  public DigitalOut
            ,  public PortIn
            ,  public PortOut
{
public:
    explicit MCP()=delete;
    explicit MCP(const MCP&)=delete;
    explicit MCP(MCP&&)=delete;
    MCP& operator=(const MCP&)=delete;
    MCP& operator=(MCP&&)=delete;
    ~MCP()=delete;
	template<typename T,typename std::enable_if_t<std::is_lvalue_reference<T>::value,int> =0>
    static bool BuildDigitalInOutPipe(T&& t){
        *MCPBase::getBuilder() = &t;
        return MCPDirector::generateInOutPipe(*MCPBase::getBuilder());
    }
    template<typename T,typename std::enable_if_t<!std::is_lvalue_reference<T>::value,int> =0>
    static bool BuildDigitalInOutPipe(T&& t){
        *MCPBase::getBuilder() = new std::remove_reference_t<T>(std::forward<T>(t));
        return MCPDirector::generateInOutPipe(*MCPBase::getBuilder());
    }
    static MCPBase& Builder()
	{
        return **MCPBase::getBuilder();
    }
private:

};