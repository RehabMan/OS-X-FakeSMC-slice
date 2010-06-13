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
