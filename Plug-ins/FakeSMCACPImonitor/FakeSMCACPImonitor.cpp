/* 
 *  FakeSMC ACPI monitor plugin
 *  Created by Slice 26.05.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCACPImonitor.h"

#define kTimeoutMSecs 1000
//static int TZFan[5];
static int SMCx[30];

bool CompareKeys(const char* key1, const char* key2)
{
	return ((key1[0] == key2[0]) && (key1[1] == key2[1]) && (key1[2] == key2[2]) && (key1[3] == key2[3]));
}

static void Update(SMCData node)
{	
	short value;
	// FANs
	if(CompareKeys(node->key, "F0Ac") || CompareKeys(node->key, "F1Ac") || CompareKeys(node->key, "F2Ac") || CompareKeys(node->key, "F3Ac") || CompareKeys(node->key, "F4Ac") || CompareKeys(node->key, "F5Ac"))
	{
		UInt8 num = node->key[1] - 48;
		
		value = SMCx[num] * 40; //iStat fix=4 ACPI_units=10?
		
		node->data[0] = value>>8;
		node->data[1] = value & 0xff;;
	} else
	//Temp
		if(CompareKeys(node->key, "TC0H"))
	{
		node->data[0] = SMCx[10];
		node->data[1] = 0;
	} else if (CompareKeys(node->key, "TN0H")) {
		node->data[0] = SMCx[11];
		node->data[1] = 0;
		
	} else if (CompareKeys(node->key, "TW0P")) {
		node->data[0] = SMCx[12];
		node->data[1] = 0;
	} else if (CompareKeys(node->key, "Th0H")) {
		node->data[0] = SMCx[13];
		node->data[1] = 0;
	} else if (CompareKeys(node->key, "Th1H")) {
		node->data[0] = SMCx[14];
		node->data[1] = 0;
	} else 
		//Voltage
		if(CompareKeys(node->key, "VCAC")) {
			node->data[0] = SMCx[20];
			node->data[1] = 0;			
		} else if (CompareKeys(node->key, "Vp0C")) {
			node->data[0] = SMCx[21];
			node->data[1] = 0;
		} else if (CompareKeys(node->key, "Vp1C")) {
			node->data[0] = SMCx[22];
			node->data[1] = 0;
		} else if (CompareKeys(node->key, "Vp2C")) {
			node->data[0] = SMCx[23];
			node->data[1] = 0;
			// Amperage
		} else if (CompareKeys(node->key, "ICAC")) {
			node->data[0] = SMCx[24];
			node->data[1] = 0;
		} else if (CompareKeys(node->key, "Ip0C")) {
			node->data[0] = SMCx[25];
			node->data[1] = 0;
		} else if (CompareKeys(node->key, "Ip1C")) {
			node->data[0] = SMCx[26];
			node->data[1] = 0;
		} else if (CompareKeys(node->key, "Ip2C")) {
			node->data[0] = SMCx[27];
			node->data[1] = 0;
			//Power
		} else if (CompareKeys(node->key, "PC0C")) {
			node->data[0] = SMCx[28];
			node->data[1] = 0;
		} else if (CompareKeys(node->key, "PC1C")) {
			node->data[0] = SMCx[29];
			node->data[1] = 0;
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
			IOLog("FakeSMC_ACPI: %s registered\n", key);
			FCount = i+1;
		}
	}
	
	value[0] = FCount;
	FakeSMCRegisterKey("FNum", 1, value, NULL);
	
	//Next step - temperature keys
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCA", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("TC0H", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "TC0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCB", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("TN0H", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "TN0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCC", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("TW0P", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "TW0P");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCD", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("Th0H", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Th0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCE", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("Th1H", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Th1H");
	}
	
	//Voltage
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCK", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("VCAC", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "VCAC");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCL", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("Vp0C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCM", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("Vp1C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp1C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCN", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("Vp2C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp2C");
	}
	//Amperage
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCO", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("ICAC", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "ICAC");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCP", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("Ip0C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCQ", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("Ip1C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip1C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCR", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("Ip2C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip2C");
	}
	//Power
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCS", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("PC0C", 2, value, &Update);
		IOLog("FakeSMC_ACPI: %s registered\n", "PC0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCT", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCRegisterKey("PC1C", 2, value, &Update);
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
	FakeSMCUnregisterKey("TC0H");
	FakeSMCUnregisterKey("TN0H");
	FakeSMCUnregisterKey("TW0P");
	FakeSMCUnregisterKey("Th0H");
	FakeSMCUnregisterKey("Th1H");
	FakeSMCUnregisterKey("VCAC");
	FakeSMCUnregisterKey("Vp0C");
	FakeSMCUnregisterKey("Vp1C");
	FakeSMCUnregisterKey("Vp2C");
	FakeSMCUnregisterKey("ICAC");
	FakeSMCUnregisterKey("Ip0C");
	FakeSMCUnregisterKey("Ip1C");
	FakeSMCUnregisterKey("Ip2C");
	FakeSMCUnregisterKey("PC0C");
	FakeSMCUnregisterKey("PC1C");
	//	FakeSMCUnregisterKey("FNum");
	//	FakeSMCUnregisterKey("FNum");
	//	FakeSMCUnregisterKey("FNum");
	//	FakeSMCUnregisterKey("FNum");
	
	super::stop(provider);
}

void ACPImonitor::free ()
{
	super::free ();
}
