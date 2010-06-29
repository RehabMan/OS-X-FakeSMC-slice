/*
 *  FanController.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 27/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "FanController.h"

void FanController::TimerEvent()
{
	if (!m_Active) 
		return;
	
	if (m_Input != 0xff)
	{
		UInt8 t = ReadTemperature(m_Input);
		
		if (t > 0 && t != m_LastValue)
		{
			if (t < m_StartTemperature && m_LastValue > m_StartTemperature)
			{
				ForceFan(m_Index, 0);
			}
			else if (t > m_HighTemperature && m_LastValue < m_HighTemperature)
			{
				ForceFan(m_Index, m_HighThrottle);
			}
			else
			{
				ForceFan(m_Index, m_StartThrottle + ((t - m_StartTemperature) * m_Multiplier));
			}
		}
		
		m_LastValue = t;
	}
	else if (m_StartThrottle != m_LastValue ) 
	{
		m_LastValue = m_StartThrottle;
		ForceFan(m_Index, m_StartThrottle);
	}

}