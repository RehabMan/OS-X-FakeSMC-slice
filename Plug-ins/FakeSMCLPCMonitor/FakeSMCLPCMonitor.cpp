/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
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
	
	if(superio == NULL) {
		it87 = new IT87x();
		
		DebugLog("Probing ITE...");
		if (it87->Probe())
			superio=it87;
		else {
			delete superio;
			//why don't delete superio first???
			winbond = new Winbond();
			
			DebugLog("Probing Winbond...");
			if (winbond->Probe())
				superio=winbond;
			else {
				delete winbond;
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
	SInt8 hsOffset=-1, nbOffset=-1;
	if(it87)
	{
		OSNumber * currentKey = NULL;
		//get northbridge temperature number from plist
		currentKey = OSDynamicCast(OSNumber, getProperty("ITE_NORTHBRIDGE_TEMPERATURE_OFFSET"));
		if (currentKey) {
			nbOffset=currentKey->unsigned8BitValue();
			DebugLog("ITE_NORTHBRIDGE_TEMPERATURE_OFFSET=%d\n",nbOffset);
		}
		
		//get CPU Heatsink temperature number from plist
		currentKey = OSDynamicCast(OSNumber, getProperty("ITE_HEATSINK_TEMPERATURE_OFFSET"));
		if (currentKey) {
			hsOffset=currentKey->unsigned8BitValue();
			IOLog("ITE_HEATSINK_TEMPERATURE_OFFSET=%d\n",hsOffset);
		}
		
		it87->IT87x::SetTemperatureOffsets(hsOffset, nbOffset);
	}
	if (superio)
		superio->Init();
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