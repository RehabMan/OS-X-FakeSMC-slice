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
private:
	ITE*	m_Provider;
	UInt8	m_LastTemperature;
	float	m_Multiplier;
	void	ForcePWM(UInt8 value) 
	{ 
		UInt8 pwm = 127.0f * value / 100.0f;
		
		//InfoLog("Forcing Fan #%d to 0x%x", m_Index, pwm);
		
		((ITE*)m_Provider)->WriteByte(ITE_SMARTGUARDIAN_PWM_CONTROL[m_Index], pwm); 
	};
public:
	ITEFanController(ITE* provider, UInt8 fan_index, UInt8 temp_input, UInt8 start_temp, UInt8 start_throttle, UInt8 high_temp, UInt8 high_throttle) : FanController(fan_index, temp_input, start_temp, start_throttle, high_temp, high_throttle)
	{
		m_Provider = provider;
		m_LastTemperature = provider->ReadTemperature(temp_input);
		m_Multiplier = float(high_throttle - start_throttle) / float(high_temp - start_temp);
		
		InfoLog("Activating software control on Fan #%d", fan_index);
	};
	
	virtual void TimerEvent();
};