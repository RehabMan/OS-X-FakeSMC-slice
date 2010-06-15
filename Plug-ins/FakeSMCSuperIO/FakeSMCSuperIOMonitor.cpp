/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 */

#include "FakeSMCSuperIOMonitor.h"

#define super IOService
OSDefineMetaClassAndStructors(FakeSMCSuperIOMonitor, IOService)

bool setNumber(const char * symbol, UInt32 value, OSDictionary * dictionary)
{
	OSNumber * number = OSNumber::withNumber(value, 32);
	
	if (number)
	{
		dictionary->setObject(symbol, number);
		return true;
	}
	
	return false;
}

bool setString(const char * symbol, char * value, OSDictionary * dictionary)
{
	OSString * string = OSString::withCString(value);
	
	if (string)
	{
		dictionary->setObject(symbol, string);
		return true;
	}
	
	return false;
}

bool setString(const char * symbol, const char * value, OSDictionary * dictionary)
{
	OSString * string = OSString::withCString(value);
	
	if (string)
	{
		dictionary->setObject(symbol, string);
		return true;
	}
	
	return false;
}

bool setString(const char * symbol, const OSSymbol * value, OSDictionary * dictionary)
{
	dictionary->setObject(symbol, value);
	return true;
}

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
		superio = new ITE();
			
		if(!superio->Probe())
		{
			delete superio;
			
			superio = new Winbond();
					
			if(!superio->Probe())
			{
				delete superio;
				
				superio = new Fintek();
								
				if(!superio->Probe())
				{
					InfoLog("No supported Super I/O chip found!");
					return 0;
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