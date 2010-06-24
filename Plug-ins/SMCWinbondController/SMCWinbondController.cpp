/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 */

#include "SMCWinbondController.h"

#define super IOService
OSDefineMetaClassAndStructors(FakeSMCSuperIOMonitor, IOService)

bool FakeSMCSuperIOMonitor::init(OSDictionary *properties)
{
	//DebugLog("Initializing...");
	
    super::init(properties);
	
	return true;
}

IOService* FakeSMCSuperIOMonitor::probe(IOService *provider, SInt32 *score)
{
	//DebugLog("Probing...");
	
	if (super::probe(provider, score) != this) return false;
	
	superio = new Winbond();
	
	if(!superio->Probe())
		return false;
			
	InfoLog("Found %s Super I/O chip", superio->GetModelName());

	superio->LoadConfiguration(this);
	
	return this;
}

bool FakeSMCSuperIOMonitor::start(IOService * provider)
{
	//DebugLog("Starting...");
	
	if (!super::start(provider)) return false;
	
	if(superio)
		superio->Init();
	else 
		return false;
	
	return true;
}

void FakeSMCSuperIOMonitor::stop (IOService* provider)
{
	//DebugLog("Stopping...");
	
	if(superio)
		superio->Finish();

	super::stop(provider);
}

void FakeSMCSuperIOMonitor::free ()
{
	//DebugLog("Freeing...");
	
	delete superio;
	
	super::free ();
}