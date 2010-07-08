/*
 *  Controller.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 08/07/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _CONTROLLER_H 
#define _CONTROLLER_H

#include "SuperIO.h"

class Controller : public Binding
{
public:
	virtual void TimerEvent() 
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
	
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
};

#endif