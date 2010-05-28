/* 
 *  FakeSMC ThermalZone plugin
 *  Created by Slice 21.05.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCTZplugin.h"
#include "fakesmc.h"

#define kTimeoutMSecs 1000
static int TZtemp;

static void Update(const char* key, char* data)
{	
	data[0] = TZtemp;
	data[1] = 0;
}

#define super IOService
OSDefineMetaClassAndStructors(TZPlugin, IOService)

IOService* TZPlugin::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;

	return this;
}

bool TZPlugin::start(IOService * provider)
{
	if (!provider || !super::start(provider)) return false;
	
	TZDevice = (IOACPIPlatformDevice *) provider;
	

	
	char value[2];
	for (int i=0; i<TCount; i++) 
	{
		
		value[0] = TZtemp;
		value[1] = 0;
		
		char key[5];
		
		snprintf(key, 5, "TN%dP", i);
		
		FakeSMCAddKeyCallback(key, "sp78", 0x02, value, &Update);
						   //OSMemberFunctionCast(PluginCallback, this, &TZPlugin::Update));
		IOLog("FakeSMC_TZ: %s registered\n", key);
	}
	
	//Here is Fan on TZ	
	for (int i=0; i<FCount; i++) 
	{
		
		value[0] = 10;
		value[1] = 0;
		
		char key[5];
		
		snprintf(key, 5, "FN%dP", i);
		
		FakeSMCAddKeyCallback(key, "sp78", 0x02, value, &Update);
		IOLog("FakeSMC_TZ: %s registered\n", key);
	}
	

	registerService(0);
	TZWorkLoop = getWorkLoop();
	TZPollTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &TZPlugin::poller));
	if (!TZWorkLoop || !TZPollTimer || (kIOReturnSuccess != TZWorkLoop->addEventSource(TZPollTimer))) return false;
	poller();
	

	return true;	
}

IOReturn TZPlugin::poller( void )
{
	IOReturn ret = kIOReturnSuccess;
	UInt32 tmp;
	if (TZDevice) {
		if (kIOReturnSuccess == TZDevice->evaluateInteger("_TMP", &tmp))
			TZtemp = ((int)tmp - 2732) / 10;
	}
	else {
		ret = kIOReturnNoDevice;
	}
	TZPollTimer->setTimeoutMS(kTimeoutMSecs);	
	return(ret);
}

bool TZPlugin::init(OSDictionary *properties)
{
    super::init(properties);
	TCount = 1;
	FCount = 0;
	return true;
}

void TZPlugin::stop (IOService* provider)
{
	char key[5];
	for (int i=0; i<TCount; i++) 
	{
		snprintf(key, 5, "TN%dP", i);
		
		FakeSMCRemoveKeyCallback(key);
	}
	
	for (int i=0; i<FCount; i++) 
	{
		snprintf(key, 5, "FN%dP", i);
		FakeSMCRemoveKeyCallback(key);
	}
	
	super::stop(provider);
}

void TZPlugin::free ()
{
	super::free ();
}
