/*
 *  SmartFanIIIController.h
 *  FakeSMCSuperIOMonitor
 *
 *  Created by Mozodojo on 13/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef SmartGuardianController_h 
#define SmartGuardianController_h

#include "Binding.h"
#include "Winbond.h"

class SmartFanIIIController : public Binding 
{
private:
	Winbond*	m_Provider;
	UInt8	m_Offset;
	UInt8	m_Index;
	
	UInt8	m_DefaultForcePWM;
	UInt8	m_DefaultStartPWM;
	UInt16	m_Maximum;
	UInt16	m_Minimum;
	
	void	ForcePWM(UInt8 slope);
public:
	SmartFanIIIController(Winbond* provider, UInt8 offset, UInt8 index) : Binding()
	{
		m_Provider = provider;
		m_Offset = offset;
		m_Index = index;
		
		Initialize();
	};
	
	void			Initialize();
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

#endif
