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

class WinbondFanController : public FanController
{
public:
	WinbondFanController(Winbond* provider, UInt8 index) : FanController(provider, index)
	{ 
	};
	
	virtual UInt8 ReadTemperature(UInt8 index)
	{
		return m_Provider->ReadTemperature(index);
	};
	
	virtual void ForceFan(UInt8 index, UInt8 value) 
	{ 
		if (m_Provider->IsFanVoltageControlled())
		{
			m_Provider->WriteByte(0, WINBOND_FAN_OUTPUT[index], 64.0f * value / 100.0f);
		}
		else
		{
			m_Provider->WriteByte(0, WINBOND_FAN_OUTPUT[index], 255.0f * value / 100.0f);
		}
	};	
};