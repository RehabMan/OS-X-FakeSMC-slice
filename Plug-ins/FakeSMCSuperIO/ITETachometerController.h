/*
 *  ITETachometerController.h
 *  FakeSMCSuperIOMonitor
 *
 *  Created by Mozodojo on 13/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef ITETachometerController_h 
#define ITETachometerController_h

#include "Binding.h"
#include "ITE.h"

const UInt8 ITE_SMARTGUARDIAN_FORCE_PWM[5] = { 0x15, 0x16, 0x17, 0x88, 0x89 };
const UInt8 ITE_SMARTGUARDIAN_START_PWM[5] = { 0x63, 0x6b, 0x73, 0x93, 0x9b };
const UInt8 ITE_SMARTGUARDIAN_CONTROL[5] = { 0x64, 0x6c, 0x74, 0x94, 0x9c };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_OFF[5] = { 0x60, 0x68, 0x70, 0x90, 0x98 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_START[5] = { 0x61, 0x69, 0x71, 0x91, 0x99 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_FULL[5] = { 0x62, 0x6a, 0x72, 0x95, 0x9a };

class ITETachometerController : public Binding 
{
private:
	UInt16	m_Address;
	UInt8	m_Offset;
	UInt8	m_Index;
	
	UInt8	m_DefaultForcePWM;
	UInt8	m_DefaultStartPWM;
	UInt16	m_Maximum;
	
	void	ForcePWM(UInt8 slope);
public:
	ITETachometerController(UInt16 address, UInt8 offset, UInt8 index) : Binding()
	{
		m_Address = address;
		m_Offset = offset;
		m_Index = index;
		
		Initialize();
	};
	
	void			Initialize();
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

#endif
