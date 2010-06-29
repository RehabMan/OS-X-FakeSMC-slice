/*
 *  FanController.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 27/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _FANCONTROLLER_H 
#define _FANCONTROLLER_H

#include "BaseDefinitions.h"
#include "SuperIO.h"
#include "Sensor.h"

class FanController : public Controller 
{
private:
	bool	m_Active;
	UInt8	m_Input;
	UInt8	m_Index;
	float	m_Multiplier;
	UInt8	m_LastValue;
	UInt8	m_StartTemperature;
	UInt8	m_StartThrottle;
	UInt8	m_HighThrottle;
	UInt8	m_HighTemperature;
	
protected:
	SuperIO*	m_Provider;
	
public:
	FanController(SuperIO* provider, UInt8 index, OSDictionary* configuration)
	{
		m_Provider = provider;
		m_Index = index;
			
		if (configuration)
		{
			OSString* tempSource = OSDynamicCast(OSString, configuration->getObject("Temperature Source"));
			OSNumber* startTemp = OSDynamicCast(OSNumber, configuration->getObject("Start Temperature, ℃"));
			OSNumber* highTemp = OSDynamicCast(OSNumber, configuration->getObject("High Temperature, ℃"));
			OSNumber* startThrottle = OSDynamicCast(OSNumber, configuration->getObject("Start Throttle, %"));
			OSNumber* highThrottle = OSDynamicCast(OSNumber, configuration->getObject("High Throttle, %"));
			
			const char* inputKey;
			
			if (!tempSource) 
				return;
			else if (tempSource->isEqualTo("Processor"))
				inputKey = KEY_CPU_HEATSINK_TEMPERATURE;
			else if (tempSource->isEqualTo("System"))
				inputKey = KEY_NORTHBRIDGE_TEMPERATURE;
			else
				return;
			
			m_Input = 0xff;
			
			for (Sensor* sensor = (Sensor*)m_Provider->GetBindings(); sensor; sensor = (Sensor*)sensor->Next)
			{
				if (CompareKeys(sensor->GetKey(), inputKey))
				{
					m_Input = sensor->GetIndex();
				}
			}
			
			if (!startTemp || !highTemp || highTemp->unsigned8BitValue() < startTemp->unsigned8BitValue()) 
				return;
			
			if (!startThrottle || !highThrottle || highThrottle->unsigned8BitValue() < startThrottle->unsigned8BitValue()) 
				return;
		
			m_StartTemperature = startTemp->unsigned8BitValue();
			m_StartThrottle = startThrottle->unsigned8BitValue();
			m_HighTemperature = highTemp->unsigned8BitValue();
			m_HighThrottle = highThrottle->unsigned8BitValue();
			
			CalculateMultiplier();
			
			m_Active = true;
		}
		
		InfoLog("Activating software control on Fan #%d", index);
	};
	
	virtual void	CalculateMultiplier() { m_Multiplier = float(m_HighThrottle - m_StartThrottle) / float(m_HighTemperature - m_StartTemperature); };
	virtual UInt8	ReadTemperature(__unused UInt8 index) { return 0; };
	virtual void	ForceFan(__unused UInt8 index, __unused UInt8 value) {};
	virtual void	TimerEvent();
};

#endif