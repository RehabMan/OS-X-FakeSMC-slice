/*
 *  WinbondFanController.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 27/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Winbond.h"
#include "FanController.h"

const UInt8 WINBOND_FAN_OUTPUT[]	= { 0x01, 0x03, 0x11, 0x61 };

class WinbondFanController : public FanController
{
public:
	WinbondFanController(Winbond* provider, UInt8 index, OSDictionary* configuration) : FanController(provider, index, configuration)
	{ 
	};
	
	virtual UInt8 ReadTemperature(UInt8 index)
	{
		return m_Provider->ReadTemperature(index);
	};
	
	virtual void ForceFan(UInt8 index, UInt8 value) 
	{ 
		m_Provider->WriteByte(0, WINBOND_FAN_OUTPUT[index], 255.0f * value / 100.0f);
	};	
};