/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 */

#include "FakeSMCSuperIOMonitor.h"

#define super IOService
OSDefineMetaClassAndStructors(SuperIOMonitorPlugin, IOService)

bool SuperIOMonitorPlugin::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
    super::init(properties);
	
	return true;
}

IOService* SuperIOMonitorPlugin::probe(IOService *provider, SInt32 *score)
{
	DebugLog("Probing...");
	
	if (super::probe(provider, score) != this) return 0;
	
	if(superio == NULL)
	{
		superio = new IT87x();
		
		DebugLog("Probing ITE...");
		
		if(!superio->Probe())
		{
			delete superio;
			
			superio = new Winbond();
			
			DebugLog("Probing Winbond...");
			
			if(!superio->Probe())
			{
				InfoLog("No supported Super I/O chip found!");
				return 0;
			}
		}
	}
	
	InfoLog("Found %s Super I/O chip", superio->GetModelName());

	superio->LoadConfiguration(this);
	
	return this;
}

bool SuperIOMonitorPlugin::start(IOService * provider)
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

void SuperIOMonitorPlugin::stop (IOService* provider)
{
	DebugLog("Stoping...");
	
	if(superio)
		superio->Finish();

	super::stop(provider);
}

void SuperIOMonitorPlugin::free ()
{
	DebugLog("Freeing...");
	
	delete superio;
	
	super::free ();
}