/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 *	This code includes parts of original Open Hardware Monitor code
 *	Copyright © 2010 Michael Möller. All Rights Reserved.
 *
 */

#include "FakeSMCLPCMonitor.h"
#include <IOKit/IOLib.h>

#define super IOService
OSDefineMetaClassAndStructors(LPCMonitorPlugin, IOService)

bool LPCMonitorPlugin::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
    super::init(properties);
	
	return true;
}

IOService* LPCMonitorPlugin::probe(IOService *provider, SInt32 *score)
{
	DebugLog("Probing...");
	
	if (super::probe(provider, score) != this) return 0;
	
	if(superio == NULL)
	{
		superio = new IT87x();
		
		DebugLog("Probing ITE...");
		
		if(!superio->Probe())
		{
			superio = new Winbond();
			
			DebugLog("Probing Winbond...");
			
			if(!superio->Probe())
			{
				return 0;
			}
		}
	}
	
	InfoLog("found %s", superio->GetModelNameString());

	return this;
}

bool LPCMonitorPlugin::start(IOService * provider)
{
	DebugLog("Starting...");
	
	if (!super::start(provider)) return false;
	
	if(superio)
	{
		superio->Init();
	}
	else 
	{
		return false;
	}

	return true;
}

void LPCMonitorPlugin::stop (IOService* provider)
{
	DebugLog("Stoping...");
	
	if(superio)
		superio->Finish();

	super::stop(provider);
}

void LPCMonitorPlugin::free ()
{
	DebugLog("Freeing...");
	
	super::free ();
}