/*
 *  FanController.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 27/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "FanController.h"

bool FanController::UpdateConfiguration(OSDictionary* configuration)
{
	if (!configuration)
		return false;

	OSString* tempSource = OSDynamicCast(OSString, configuration->getObject("Temperature Source"));
	OSNumber* startTemp = OSDynamicCast(OSNumber, configuration->getObject("Start Temperature, ℃"));
	OSNumber* highTemp = OSDynamicCast(OSNumber, configuration->getObject("High Temperature, ℃"));
	OSNumber* startThrottle = OSDynamicCast(OSNumber, configuration->getObject("Start Throttle, %"));
	OSNumber* highThrottle = OSDynamicCast(OSNumber, configuration->getObject("High Throttle, %"));
	
	const char* inputKey;
	
	if (!tempSource) 
	{
		Deactivate();
		return false;
	}
	else if (tempSource->isEqualTo("Processor"))
	{
		inputKey = KEY_CPU_HEATSINK_TEMPERATURE;
	}
	else if (tempSource->isEqualTo("System"))
	{
		inputKey = KEY_NORTHBRIDGE_TEMPERATURE;
	}
	else
	{
		Deactivate();
		return false;
	}
	
	for (Binding* sensor = m_Provider->GetSensors(); sensor; sensor = sensor->Next)
	{
		if (CompareKeys(((Sensor*)sensor)->GetKey(), inputKey))
		{
			m_Input = ((Sensor*)sensor);
		}
	}
	
	if (!startTemp || !highTemp || highTemp->unsigned8BitValue() < startTemp->unsigned8BitValue())
	{
		Deactivate();
		return false;
	}
	
	if (!startThrottle || !highThrottle || highThrottle->unsigned8BitValue() < startThrottle->unsigned8BitValue()) 
	{
		Deactivate();
		return false;
	}
		
	m_StartTemperature = startTemp->unsigned8BitValue();
	m_StartThrottle = startThrottle->unsigned8BitValue();
	m_HighTemperature = highTemp->unsigned8BitValue();
	m_HighThrottle = highThrottle->unsigned8BitValue();
		
	CalculateMultiplier();
	
	Activate();
		
	return true;
}

void FanController::TimerEvent()
{
	if (!m_Active) 
		return;
	
	if (m_Input)
	{
		SInt32 t = m_Input->GetValue();
		
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
	else if (m_HighThrottle != m_LastValue ) 
	{
		m_LastValue = m_StartThrottle;
		ForceFan(m_Index, m_StartThrottle);
	}

}