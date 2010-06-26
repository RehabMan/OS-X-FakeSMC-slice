/*
 *  ITEFanController.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 26/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "ITEFanController.h"

void ITEFanController::TimerEvent()
{
	UInt8 t = m_Provider->ReadTemperature(m_Input);
	
	if (t > 0 && t != m_LastTemperature)
	{
		if (t < m_StartTemperature && m_LastTemperature > m_StartTemperature)
		{
			ForcePWM(0);
		}
		else if (t > m_HighTemperature && m_LastTemperature < m_HighTemperature)
		{
			ForcePWM(m_HighThrottle);
		}
		else
		{
			ForcePWM(m_StartThrottle + ((t - m_StartTemperature) * m_Multiplier));
		}
	}
		 
	m_LastTemperature = t;
}