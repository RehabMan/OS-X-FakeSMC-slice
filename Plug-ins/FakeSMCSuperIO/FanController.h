/*
 *  FanController.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 26/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _FANCONTROLLER_H 
#define _FANCONTROLLER_H

#include "BaseDefinitions.h"
#include "SuperIO.h"

class FanController : public Controller 
{
protected:
	UInt8		m_Index;
	UInt8		m_Input;
	UInt8		m_StartTemperature;
	UInt8		m_StartThrottle;
	UInt8		m_HighThrottle;
	UInt8		m_HighTemperature;
public:
	FanController(UInt8 fan_index, UInt8 temp_input, UInt8 start_temp, UInt8 start_throttle, UInt8 high_temp, UInt8 high_throttle)
	{
		m_Index = fan_index;
		m_Input = temp_input;
		m_StartTemperature = start_temp;
		m_HighTemperature = high_temp;
		m_StartThrottle = start_throttle;
		m_HighThrottle = high_throttle;
	};
	
	virtual void TimerEvent() {};
};

#endif