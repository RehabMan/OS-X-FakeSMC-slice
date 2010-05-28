/* 
 *  FakeSMC ACPI monitor plugin
 *  Created by Slice 26.05.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCACPImonitor.h"
#include "fakesmc.h"

#define kTimeoutMSecs 1000
//static int TZFan[5];
static int SMCx[30];

static void Update(const char* key, char* data)
{	
	short value;
	// FANs
	if(CompareKeys(key, "F0Ac") || CompareKeys(key, "F1Ac") || CompareKeys(key, "F2Ac") || CompareKeys(key, "F3Ac") || CompareKeys(key, "F4Ac") || CompareKeys(key, "F5Ac"))
	{
		UInt8 num = key[1] - 48;
		
		value = SMCx[num] * 10; // * 40; //iStat fix=4 ACPI_units=10? // mozo: need to check now with fpe2 type, like it should
		
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	} else
	//Temp
		if(CompareKeys(key, "TC0H"))
	{
		data[0] = SMCx[10];
		data[1] = 0;
	} else if (CompareKeys(key, "TN0H")) {
		data[0] = SMCx[11];
		data[1] = 0;
		
	} else if (CompareKeys(key, "TW0P")) {
		data[0] = SMCx[12];
		data[1] = 0;
	} else if (CompareKeys(key, "Th0H")) {
		data[0] = SMCx[13];
		data[1] = 0;
	} else if (CompareKeys(key, "Th1H")) {
		data[0] = SMCx[14];
		data[1] = 0;
	} else 
		//Voltage
		if(CompareKeys(key, "VCAC")) {
			data[0] = SMCx[20];
			data[1] = 0;			
		} else if (CompareKeys(key, "Vp0C")) {
			data[0] = SMCx[21];
			data[1] = 0;
		} else if (CompareKeys(key, "Vp1C")) {
			data[0] = SMCx[22];
			data[1] = 0;
		} else if (CompareKeys(key, "Vp2C")) {
			data[0] = SMCx[23];
			data[1] = 0;
			// Amperage
		} else if (CompareKeys(key, "ICAC")) {
			data[0] = SMCx[24];
			data[1] = 0;
		} else if (CompareKeys(key, "Ip0C")) {
			data[0] = SMCx[25];
			data[1] = 0;
		} else if (CompareKeys(key, "Ip1C")) {
			data[0] = SMCx[26];
			data[1] = 0;
		} else if (CompareKeys(key, "Ip2C")) {
			data[0] = SMCx[27];
			data[1] = 0;
			//Power
		} else if (CompareKeys(key, "PC0C")) {
			data[0] = SMCx[28];
			data[1] = 0;
		} else if (CompareKeys(key, "PC1C")) {
			data[0] = SMCx[29];
			data[1] = 0;
		}			
}



#define super IOService
OSDefineMetaClassAndStructors(ACPImonitor, IOService)

IOService* ACPImonitor::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	
	return this;
}

bool ACPImonitor::start(IOService * provider)
{
	if (!provider || !super::start(provider)) return false;
	
	TZDevice = (IOACPIPlatformDevice *) provider;
	
	char key[5];
	char value[2];
	FCount = 0;
	UInt32 tmp;
	//Here is Fan in ACPI	
	for (int i=0; i<10; i++) 
	{
		snprintf(key, 5, "SMC%d", i);
		if (kIOReturnSuccess == TZDevice->evaluateInteger(key, &tmp)){		
			value[0] = 0x0;
			value[1] = 0x0;
			snprintf(key, 5, "F%dAc", i);
			FakeSMCAddKeyCallback(key, "fpe2", 2, value, &Update);
			IOLog("FakeSMC_ACPI: %s registered\n", key);
			FCount = i+1;
		}
	}
	
	value[0] = FCount;
	FakeSMCAddKey("FNum", 1, value);
	
	//Next step - temperature keys
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCA", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("TC0H", "sp78", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "TC0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCB", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("TN0H", "sp78", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "TN0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCC", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("TW0P", "sp78", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "TW0P");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCD", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("Th0H", "sp78", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Th0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCE", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("Th1H", "sp78", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Th1H");
	}
	
	//Voltage
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCK", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("VCAC", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "VCAC");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCL", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("Vp0C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCM", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("Vp1C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp1C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCN", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("Vp2C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp2C");
	}
	//Amperage
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCO", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("ICAC", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "ICAC");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCP", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("Ip0C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCQ", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("Ip1C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip1C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCR", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("Ip2C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip2C");
	}
	
	//Power
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCS", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("PC0C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "PC0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCT", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKeyCallback("PC1C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "PC1C");
	}	
	
	registerService(0);
	TZWorkLoop = getWorkLoop();
	TZPollTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &ACPImonitor::poller));
	if (!TZWorkLoop || !TZPollTimer || (kIOReturnSuccess != TZWorkLoop->addEventSource(TZPollTimer))) return false;
	poller();
	return true;	
}

IOReturn ACPImonitor::poller( void )
{
	IOReturn ret = kIOReturnSuccess;
	UInt32 tmp;
	SInt8 t2;
	char key[4];
	if (TZDevice) {
		//Here is Fan in ACPI	
		for (int i=0; i<FCount; i++) 
		{
			snprintf(key, 5, "SMC%d", i);
			if (kIOReturnSuccess == TZDevice->evaluateInteger(key, &tmp)){	
				t2 = tmp;
				SMCx[i] = (int)(~t2);
			}
		}
		
		//Next step - temperature keys
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCA", &tmp)){		
			SMCx[10] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCB", &tmp)){		
			SMCx[11] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCC", &tmp)){		
			SMCx[12] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCD", &tmp)){		
			SMCx[13] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCE", &tmp)){		
			SMCx[14] = tmp;
		}
		
		//Voltage
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCK", &tmp)){		
			SMCx[20] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCL", &tmp)){		
			SMCx[21] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCM", &tmp)){		
			SMCx[22] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCN", &tmp)){		
			SMCx[23] = tmp;
		}
		//Amperage
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCO", &tmp)){		
			SMCx[24] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCP", &tmp)){		
			SMCx[25] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCQ", &tmp)){		
			SMCx[26] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCR", &tmp)){		
			SMCx[27] = tmp;
		}
		//Power
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCS", &tmp)){		
			SMCx[28] = tmp;
		}
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCT", &tmp)){		
			SMCx[29] = tmp;
		}	
		
	}
	else {
		ret = kIOReturnNoDevice;
	}
	TZPollTimer->setTimeoutMS(kTimeoutMSecs);	
	return(ret);
}

bool ACPImonitor::init(OSDictionary *properties)
{
    super::init(properties);
	FCount = 0;
	return true;
}

void ACPImonitor::stop (IOService* provider)
{
	char key[5];
	for (int i=0; i<FCount; i++) 
	{
		snprintf(key, 5, "F%dAc", i);
		FakeSMCRemoveKeyCallback(key);
	}
	FakeSMCRemoveKeyCallback("TC0H");
	FakeSMCRemoveKeyCallback("TN0H");
	FakeSMCRemoveKeyCallback("TW0P");
	FakeSMCRemoveKeyCallback("Th0H");
	FakeSMCRemoveKeyCallback("Th1H");
	FakeSMCRemoveKeyCallback("VCAC");
	FakeSMCRemoveKeyCallback("Vp0C");
	FakeSMCRemoveKeyCallback("Vp1C");
	FakeSMCRemoveKeyCallback("Vp2C");
	FakeSMCRemoveKeyCallback("ICAC");
	FakeSMCRemoveKeyCallback("Ip0C");
	FakeSMCRemoveKeyCallback("Ip1C");
	FakeSMCRemoveKeyCallback("Ip2C");
	FakeSMCRemoveKeyCallback("PC0C");
	FakeSMCRemoveKeyCallback("PC1C");
	
	super::stop(provider);
}

void ACPImonitor::free ()
{
	super::free ();
}
