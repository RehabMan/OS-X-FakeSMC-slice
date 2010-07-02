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
		if (m_Provider->FanControlEnabled() && index < 5)
		{
			// Enable Manual Control
			UInt8 config = ((Winbond*)m_Provider)->ReadByte(0, WINBOND_FAN_CONTROL[index]);
			((Winbond*)m_Provider)->WriteByte(0, WINBOND_FAN_CONTROL[index], config & (1<<WINBOND_FAN_CONTROL_BIT[index]));
			
			// PWM or Voltage
			config = ((Winbond*)m_Provider)->ReadByte(0, WINBOND_FAN_MODE[index]);
			((Winbond*)m_Provider)->WriteByte(0, WINBOND_FAN_MODE[index], ((UInt8)m_Provider->FanVoltageControlled()) << WINBOND_FAN_MODE_BIT[index]);
		}
		else 
		{
			Deactivate();
		}
	};
	
	virtual UInt8 ReadTemperature(UInt8 index)
	{
		return m_Provider->ReadTemperature(index);
	};
	
	virtual void ForceFan(UInt8 index, UInt8 value) 
	{ 
		if (m_Provider->FanVoltageControlled())
		{
			value = 64.0f * value / 100.0f;
			
			m_Provider->WriteByte(0, WINBOND_FAN_OUTPUT[index], value << 2);
		}
		else
		{
			m_Provider->WriteByte(0, WINBOND_FAN_OUTPUT[index], 255.0f * value / 100.0f);
		}
	};	
};