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

#include "FakeSMC.h"
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
	FanController(SuperIO* provider, UInt8 index)
	{
		m_Provider = provider;
		m_Index = index;
		
		if (OSDictionary* fanControlConfig = OSDynamicCast(OSDictionary, m_Provider->GetService()->getProperty("Fan Control")))
		{
			char key[5];
		
			snprintf(key, 5, "Fan%d", m_Index);
			
			if (OSDictionary* fanConfig = OSDynamicCast(OSDictionary, fanControlConfig->getObject(key)))
			{
				UpdateConfiguration(fanConfig);
			}
		}
	};
	
	void Activate() 
	{ 
		InfoLog("Activating software control on Fan #%d", m_Index);
		m_Active = true; 
	};
	
	void Deactivate() 
	{
		InfoLog("Software control on Fan #%d deactivated", m_Index);
		m_Active = false; 
	};
	
	virtual void CalculateMultiplier() 
	{ 
		m_Multiplier = float(m_HighThrottle - m_StartThrottle) / float(m_HighTemperature - m_StartTemperature); 
	};
	
	virtual bool	UpdateConfiguration(OSDictionary* configuration);
	virtual UInt8	ReadTemperature(__unused UInt8 index) { return 0; };
	virtual void	ForceFan(__unused UInt8 index, __unused UInt8 value) {};
	virtual void	TimerEvent();
};

#endif