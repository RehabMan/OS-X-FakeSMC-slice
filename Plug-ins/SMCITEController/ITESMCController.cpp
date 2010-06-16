/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 */

#include "ITESMCController.h"

#define super IOService
OSDefineMetaClassAndStructors(TheOnlyWorkingITESMCController, IOService)

bool TheOnlyWorkingITESMCController::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
    super::init(properties);
	
	return true;
}

IOService* TheOnlyWorkingITESMCController::probe(IOService *provider, SInt32 *score)
{
	DebugLog("Probing...");
	
	if (super::probe(provider, score) != this) return 0;
	
	if(!superio)
	{
		superio = new ITE();
			
		if(!superio->Probe())
			delete superio;			
	}
	
	InfoLog("Found %s Super I/O chip", superio->GetModelName());

	superio->LoadConfiguration(this);
	
	return this;
}

bool TheOnlyWorkingITESMCController::start(IOService * provider)
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

void TheOnlyWorkingITESMCController::stop (IOService* provider)
{
	DebugLog("Stoping...");
	
	if(superio)
		superio->Finish();

	super::stop(provider);
}

void TheOnlyWorkingITESMCController::free ()
{
	DebugLog("Freeing...");
	
	delete superio;
	
	super::free ();
}