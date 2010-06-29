/*
 *  ITEFanController.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 26/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "ITE.h"
#include "FanController.h"

class ITEFanController : public FanController
{
public:
	ITEFanController(ITE* provider, UInt8 index, OSDictionary* configuration) : FanController(provider, index, configuration)
	{
	};
	
	virtual UInt8 ReadTemperature(UInt8 index)
	{
		return m_Provider->ReadTemperature(index);
	};
	
	virtual void ForceFan(UInt8 index, UInt8 value) 
	{ 
		m_Provider->WriteByte(ITE_SMARTGUARDIAN_PWM_CONTROL[index], 127.0f * value / 100.0f);
	};
};