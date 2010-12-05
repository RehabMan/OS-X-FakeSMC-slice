/*
 *  ACPIMonitor.cpp
 *  HWSensors
 *
 *  Created by mozo on 12/11/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "ACPIMonitor.h"
#include "FakeSMC.h"

#define Debug FALSE

#define LogPrefix "IntelThermal: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super IOService
OSDefineMetaClassAndStructors(ACPIMonitor, IOService)

bool ACPIMonitor::addSensor(const char* key, const char* type, unsigned char size)
{
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)key, (void *)type, (void *)size, (void *)this))
		return sensors->setObject(OSString::withCString(key));
	
	return 0;
}

int ACPIMonitor::addTachometer(const char* caption)
{
	for (int i = 0; i < 0x10; i++) {
		
		char name[5];
		
		snprintf(name, 5, KEY_FORMAT_FAN_SPEED, i); 
		
		if (addSensor(name, TYPE_FPE2, 2)) {
			if (caption) {
				snprintf(name, 5, KEY_FORMAT_FAN_ID, i); 
				
				if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyValue, false, (void *)name, (void *)TYPE_CH8, (void *)((UInt64)strlen(caption)), (void *)caption))
					WarningLog("error adding tachometer id value");
			}
			
			UInt8 length = 0;
			void * data = 0;
			
			IOReturn result = fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, false, (void *)KEY_FAN_NUMBER, (void *)&length, (void *)&data, 0);
			
			if (kIOReturnError == result) {
				length = 1;
				
				if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyValue, false, (void *)KEY_FAN_NUMBER, (void *)TYPE_UI8, (void *)1, (void *)&length))
					WarningLog("error adding FNum value");
			}
			else if (kIOReturnSuccess == result) {
				length = 0;
				
				bcopy(data, &length, 1);
				
				length++;
				
				if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, false, (void *)KEY_FAN_NUMBER, (void *)1, (void *)&length, 0))
					WarningLog("error updating FNum value");
			}
			else WarningLog("error reading FNum value");
			
			return i;
		}
	}
	
	return -1;
}

IOService* ACPIMonitor::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	
	return this;
}

bool ACPIMonitor::start(IOService * provider)
{
	if (!provider || !super::start(provider)) return false;
	
	if (!(fakeSMC = waitForService(serviceMatching(kFakeSMCService)))) {
		WarningLog("Can't locate fake SMC device, kext will not load");
		return false;
	}
	
	TZDevice = (IOACPIPlatformDevice *) provider;
	
	char key[5];
	char value[2];
#if __LP64__
	UInt64 tmp;
#else
	UInt32 tmp;
#endif
	
	//Here is Fan in ACPI	
	for (int i=0; i<10; i++) 
	{
		snprintf(key, 5, "SMC%d", i);
		
		if (kIOReturnSuccess == TZDevice->evaluateInteger(key, &tmp)){		

			if (addTachometer(FanName[i]) > -1) {
				WarningLog("Can't add tachometer sensor, kext will not load");
				return false;
			}
		} else break;
	}

	
	//Next step - temperature keys
	if (kIOReturnSuccess == TZDevice->evaluateInteger("SMCA", &tmp)){		
		addSensor("TC0H", "sp78", 2, <#int index#>)
		value[0] = tmp;
		value[1] = 0x0;
		FakeSMCAddKey(, , 2, value, m_Binding);
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

	return true;	
}


bool ACPIMonitor::init(OSDictionary *properties)
{
    if (!super::init(properties))
		return false;
	
	if (!(sensors = OSDictionary::withCapacity(0)))
		return false;

    if (OSArray* fanIDs = OSDynamicCast(OSArray, getProperty("Fan Names"))) {
		for (UInt32 i = 0; i < 10; i++)	{
			OSString* name = OSDynamicCast(OSString, fanIDs->getObject(i)); 
			FanName[i] = name->getCStringNoCopy();
		}
    }
	else { 
		WarningLog("Can't load Fan names...");
	}
	
	return true;
}

void ACPIMonitor::stop (IOService* provider)
{
	sensors->flushCollection();
	
	/*char key[5];
	for (int i=0; i<FCount; i++) 
	{
		snprintf(key, 5, "F%dAc", i+FanOffset);
		FakeSMCRemoveKeyBinding(key);
	}
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
	FakeSMCRemoveKeyBinding("PC1C");*/
	
	super::stop(provider);
}

void ACPIMonitor::free ()
{
	sensors->release();
	
	super::free();
}
