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
	outb(RegisterPort, 0x55);
}

void SMSC::Exit()
{
	outb(RegisterPort, 0xAA);
}

bool SMSC::Probe()
{
	DebugLog("Probing SMSC...");
	
	Model = UnknownModel;
	
	for (int i = 0; i < SMSC_PORTS_COUNT; i++) 
	{
		RegisterPort	= SMSC_PORT[i];
		ValuePort		= SMSC_PORT[i] + 1;
		
		Enter();
		
        UInt16 id = ListenPortWord(SUPERIO_CHIP_ID_REGISTER);
		
		Exit();
		
		if (id != 0 && id != 0xffff)
		{
			Model = UnknownSMCS;
			return true;
		}
	}
	
	return false;
}

void SMSC::Init()
{
	
}

void SMSC::Finish()
{
	
}