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
	ITEFanController(ITE* provider, UInt8 index) : FanController(provider, index)
	{
	};
		
	virtual void ForceFan(UInt8 index, UInt8 value) 
	{ 
		((ITE*)m_Provider)->WriteByte(ITE_SMARTGUARDIAN_PWM_CONTROL[index], 127.0f * value / 100.0f);
	};
};