#pragma once
#include "app/IOExpander/MCPBase.h"
#include "Logger.h"
#include <atomic>

class MCPDirector 
{
public:	
	MCPDirector()=delete;
	~MCPDirector()=delete;
	//创建框架(稳定)
	static bool generateInOutPipe(MCPBase* builder)
	{
		*MCPBase::getBuilder()=builder;

		int retry_count=3;
		bool ok =false;
		do
		{   
			builder->power_on();
			ok = builder->begin(32,33,100000);
			ok ? delay(100) : Logger::error(__PRETTY_FUNCTION__,__LINE__);
			builder->init(0b11111100,0b11110111);	
		}while(ok==false && retry_count-- > 0);

		return ok;
	}
private:
	
};