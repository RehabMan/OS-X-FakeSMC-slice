/* 
 *  FakeSMC ACPI monitor plugin
 *  Created by Slice 26.05.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCACPImonitor.h"

//#define kTimeoutMSecs 1000

void ACPImonitor::Update(const char* key, char* data)
{	
	short value;
	UInt32 tmp;
	SInt8 t2;
	char knm[5];
		if (!TZDevice) {
			return;
		}
	// FANs
	if((key[0]=='F')&&(key[2]=='A')&&(key[3]=='c'))
	{
		UInt8 num = key[1] - 48;
		if ((num < FanOffset)||(num>(FanOffset+FCount-1))) {
			return; //it is not our FAN
		}
		snprintf(knm, 5, "SMC%d", num - FanOffset);
		if (kIOReturnSuccess == TZDevice->evaluateInteger(knm, &tmp)){	
			t2 = tmp;
			value = (int)(~t2) * 10;
		
		//value = SMCx[num] * 10; // * 40; //iStat fix=4 ACPI_units=10? // mozo: need to check now with fpe2 type, like it should
		
			data[0] = ((UInt16)value >> 6) & 0xff;
			data[1] = (value << 2) & 0xff;
			return;
		}
	} else
	//Temp
		if(CompareKeys(key, "TC0H")){
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCA", &tmp)){}
	} else if (CompareKeys(key, "TN0H")) {
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCB", &tmp)){}	
	} else if (CompareKeys(key, "TW0P")) {
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCC", &tmp)){}
	} else if (CompareKeys(key, "Th0H")) {
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCD", &tmp)){}
	} else if (CompareKeys(key, "Th1H")) {
		if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCE", &tmp)){}		
	} else 
		//Voltage
		if(CompareKeys(key, "VCAC")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCK", &tmp)){}		
		} else if (CompareKeys(key, "Vp0C")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCL", &tmp)){}		
		} else if (CompareKeys(key, "Vp1C")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCM", &tmp)){}		
		} else if (CompareKeys(key, "Vp2C")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCN", &tmp)){}		
		// Amperage
		} else if (CompareKeys(key, "ICAC")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCO", &tmp)){}		
		} else if (CompareKeys(key, "Ip0C")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCP", &tmp)){}		
		} else if (CompareKeys(key, "Ip1C")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCQ", &tmp)){}		
		} else if (CompareKeys(key, "Ip2C")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCR", &tmp)){}		
		//Power
		} else if (CompareKeys(key, "PC0C")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCS", &tmp)){}		
		} else if (CompareKeys(key, "PC1C")) {
			if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCT", &tmp)){}		
		}	
		data[0] = tmp;
		data[1] = 0;		
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
	const char*		FanName;
	
	m_Binding = new Binding(this);
	FanOffset = GetFNum();
	
	
	//Here is Fan in ACPI	
	for (int i=0; i<10; i++) 
	{
		snprintf(key, 5, "SMC%d", i);
		if (kIOReturnSuccess == TZDevice->evaluateInteger(key, &tmp)){		
			value[0] = 0x0;
			value[1] = 0x0;
			snprintf(key, 5, "F%dAc", i+FanOffset);
			FakeSMCAddKey(key, "fpe2", 2, value, m_Binding);
			IOLog("FakeSMC_ACPI: %s registered\n", key);
			FCount = i+1;
		} else break;
	}
	
	OSArray* fanIDs = OSDynamicCast(OSArray, provider->getProperty("Fan Names"));
	
	if (fanIDs) 
		fanIDs = OSArray::withArray(fanIDs);
	
    if (fanIDs) 
	{
		for (UInt32 i = 0; i < FCount; i++)
		{
			OSString* name = OSDynamicCast(OSString, fanIDs->getObject(i)); 
			FanName = name->getCStringNoCopy();
			snprintf(key, 5, "F%dID", (int)(FanOffset + i));
			FakeSMCAddKey(key, "ch8*", strlen(FanName), (char*)FanName);
		}		
		fanIDs->release();
    }
	
	
//	value[0] = FCount;
//	FakeSMCAddKey("FNum", 1, value);
	UpdateFNum(FCount);
	
	//Next step - temperature keys
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCA", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("TC0H", "sp78", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "TC0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCB", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("TN0H", "sp78", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "TN0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCC", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("TW0P", "sp78", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "TW0P");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCD", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("Th0H", "sp78", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "Th0H");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCE", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("Th1H", "sp78", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "Th1H");
	}
	
	//Voltage
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCK", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("VCAC", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "VCAC");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCL", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("Vp0C", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCM", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("Vp1C", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp1C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCN", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("Vp2C", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "Vp2C");
	}
	//Amperage
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCO", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("ICAC", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "ICAC");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCP", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("Ip0C", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCQ", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("Ip1C", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip1C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCR", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("Ip2C", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "Ip2C");
	}
	
	//Power
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCS", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("PC0C", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "PC0C");
	}
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCT", &tmp)){		
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey("PC1C", 2, value, m_Binding);
		IOLog("FakeSMC_ACPI: %s registered\n", "PC1C");
	}	
	
	registerService(0);
/*	
	TZWorkLoop = getWorkLoop();
	TZPollTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &ACPImonitor::poller));
	if (!TZWorkLoop || !TZPollTimer || (kIOReturnSuccess != TZWorkLoop->addEventSource(TZPollTimer))) return false;
	poller();
 */
	return true;	
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
		snprintf(key, 5, "F%dAc", i+FanOffset);
		FakeSMCRemoveKeyBinding(key);
	}
	UpdateFNum(-FCount);
	FakeSMCRemoveKeyBinding("TC0H");
	FakeSMCRemoveKeyBinding("TN0H");
	FakeSMCRemoveKeyBinding("TW0P");
	FakeSMCRemoveKeyBinding("Th0H");
	FakeSMCRemoveKeyBinding("Th1H");
	FakeSMCRemoveKeyBinding("VCAC");
	FakeSMCRemoveKeyBinding("Vp0C");
	FakeSMCRemoveKeyBinding("Vp1C");
	FakeSMCRemoveKeyBinding("Vp2C");
	FakeSMCRemoveKeyBinding("ICAC");
	FakeSMCRemoveKeyBinding("Ip0C");
	FakeSMCRemoveKeyBinding("Ip1C");
	FakeSMCRemoveKeyBinding("Ip2C");
	FakeSMCRemoveKeyBinding("PC0C");
	FakeSMCRemoveKeyBinding("PC1C");
	
	delete m_Binding;
	
	super::stop(provider);
}

void ACPImonitor::free ()
{
	super::free ();
}
