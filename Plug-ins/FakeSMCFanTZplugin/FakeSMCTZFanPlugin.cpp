/* 
 *  FakeSMC ThermalZone plugin
 *  Created by Slice 21.05.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCTZFanPlugin.h"
#include "fakesmc.h"

#define kTimeoutMSecs 1000
static int TZFan[5];

static void Update(const char* key, char* data)
{	
	// FANs
	if(CompareKeys(key, "F0Ac") || CompareKeys(key, "F1Ac") || CompareKeys(key, "F2Ac") || CompareKeys(key, "F3Ac") || CompareKeys(key, "F4Ac"))
	{
		UInt8 num = key[1] - 48;
		
		short value = TZFan[num];
				
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	}
}

#define super IOService
OSDefineMetaClassAndStructors(TZFanPlugin, IOService)

IOService* TZFanPlugin::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	
	return this;
}

bool TZFanPlugin::start(IOService * provider)
{
	if (!provider || !super::start(provider)) return false;
	
	TZDevice = (IOACPIPlatformDevice *) provider;
	
	char key[5];
	char value[2];
	//Here is Fan on TZ	
	for (int i=0; i<FCount; i++) 
	{
		value[0] = 0x0;
		value[1] = 0x0;
		snprintf(key, 5, "F%dAc", i);
		FakeSMCAddKeyCallback(key, "fp2e", 2, value, &Update);
		IOLog("FakeSMC_FanTZ: %s registered\n", key);
	}
	
	value[0] = FCount;
	FakeSMCAddKey("FNum", 1, value);
	
	registerService(0);
	TZWorkLoop = getWorkLoop();
	TZPollTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &TZFanPlugin::poller));
	if (!TZWorkLoop || !TZPollTimer || (kIOReturnSuccess != TZWorkLoop->addEventSource(TZPollTimer))) return false;
	poller();
	
	
	return true;	
}

IOReturn TZFanPlugin::poller( void )
{
	IOReturn ret = kIOReturnSuccess;
	UInt32 tmp;
	//	char key[4];
	if (TZDevice) {
/*		for (int i=0; i<5; i++) {
			sprintf(key, 4, "GFN%d", i);
			if (kIOReturnSuccess == TZDevice->evaluateInteger(key, &tmp))
				TZFan[i] = (int)tmp;			
		}
 */
		if (kIOReturnSuccess == TZDevice->evaluateInteger("GFAN", &tmp))
			TZFan[0] = (int)tmp;			
		
	}
	else {
		ret = kIOReturnNoDevice;
	}
	TZPollTimer->setTimeoutMS(kTimeoutMSecs);	
	return(ret);
}

bool TZFanPlugin::init(OSDictionary *properties)
{
    super::init(properties);
	FCount = 1;
	return true;
}

void TZFanPlugin::stop (IOService* provider)
{
	char key[5];
	
	for (int i=0; i<FCount; i++) 
	{
		snprintf(key, 5, "F%dAc", i);
		FakeSMCRemoveKeyCallback(key);
	}
	
	super::stop(provider);
}

void TZFanPlugin::free ()
{
	super::free ();
}
