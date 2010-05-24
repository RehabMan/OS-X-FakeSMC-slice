/* 
 *  FakeSMC ThermalZone plugin
 *  Created by Slice 21.05.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCTZFanPlugin.h"

#define kTimeoutMSecs 1000
static int TZFan[5];

bool CompareKeys(const char* key1, const char* key2)
{
	return ((key1[0] == key2[0]) && (key1[1] == key2[1]) && (key1[2] == key2[2]) && (key1[3] == key2[3]));
}

static void Update(SMCData node)
{	
	// FANs
	if(CompareKeys(node->key, "F0Ac") || CompareKeys(node->key, "F1Ac") || CompareKeys(node->key, "F2Ac") || CompareKeys(node->key, "F3Ac") || CompareKeys(node->key, "F4Ac"))
	{
		UInt8 num = node->key[1] - 48;
		
		short value = TZFan[num];
				
		node->data[0] = value;
		node->data[1] = 0;
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
		value[1] = 0xa;
		snprintf(key, 5, "F%dMn", i);
		FakeSMCRegisterKey(key, 2, value, NULL);
		
		value[0] = 0xef;
		value[1] = 0xff;
		snprintf(key, 5, "F%dMx", i);
		FakeSMCRegisterKey(key, 2, value, NULL);
		
		value[0] = 0x0;
		value[1] = 0x0;
		snprintf(key, 5, "F%dAc", i);
		FakeSMCRegisterKey(key, 2, value, &Update);
		IOLog("FakeSMC_FanTZ: %s registered\n", key);
	}
	
	value[0] = FCount;
	FakeSMCRegisterKey("FNum", 1, value, NULL);
	
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
	FakeSMCUnregisterKey("FNum");
	
	for (int i=0; i<FCount; i++) 
	{
		snprintf(key, 5, "F%dMn", i);
		FakeSMCUnregisterKey(key);
		snprintf(key, 5, "F%dMx", i);
		FakeSMCUnregisterKey(key);
		snprintf(key, 5, "F%dAc", i);
		FakeSMCUnregisterKey(key);
	}
	
	super::stop(provider);
}

void TZFanPlugin::free ()
{
	super::free ();
}
