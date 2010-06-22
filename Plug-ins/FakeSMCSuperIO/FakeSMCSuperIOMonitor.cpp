/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 */

#include "FakeSMCSuperIOMonitor.h"

#define super IOService
OSDefineMetaClassAndStructors(FakeSMCSuperIOMonitor, IOService)

bool FakeSMCSuperIOMonitor::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
    super::init(properties);
	
	return true;
}

IOService* FakeSMCSuperIOMonitor::probe(IOService *provider, SInt32 *score)
{
	DebugLog("Probing...");
	
	if (super::probe(provider, score) != this) return 0;
	
	if(superio == NULL)
	{
		superio = new Winbond();
			
		if(!superio->Probe())
		{
			delete superio;
			
			superio = new Fintek();
					
			if(!superio->Probe())
			{
				delete superio;
				
				superio = new SMSC();
				
				if(!superio->Probe())
				{
					delete superio;
				
					superio = new ITE();
									
					if(!superio->Probe())
					{
						delete superio;
						
						InfoLog("No supported Super I/O chip has been found!");
						return 0;
					}
				}
			}
		}
	}
	
	InfoLog("Found %s Super I/O chip", superio->GetModelName());

	superio->LoadConfiguration(this);
	
	return this;
}

bool FakeSMCSuperIOMonitor::start(IOService * provider)
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

void FakeSMCSuperIOMonitor::stop (IOService* provider)
{
	DebugLog("Stoping...");
	
	if(superio)
		superio->Finish();

	super::stop(provider);
}

void FakeSMCSuperIOMonitor::free ()
{
	DebugLog("Freeing...");
	
	delete superio;
	
	super::free ();
}