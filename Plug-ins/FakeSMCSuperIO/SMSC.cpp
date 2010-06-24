/*
 *  SMSC.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 22/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "SMSC.h"

void SMSC::Enter()
{
	outb(m_RegisterPort, 0x55);
}

void SMSC::Exit()
{
	outb(m_RegisterPort, 0xAA);
	outb(m_RegisterPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(m_ValuePort, 0x02);
}

bool SMSC::ProbeCurrentPort()
{
	UInt16 id = ListenPortWord(SUPERIO_CHIP_ID_REGISTER);
	
	if (id != 0 && id != 0xffff)
	{
		InfoLog("Found unsupported SMSC chip ID=0x%x", id);
		
		m_Model = UnknownSMSC;
		return true;
	}
	
	return false;
}

void SMSC::Init()
{
	
}

void SMSC::Finish()
{
	
}