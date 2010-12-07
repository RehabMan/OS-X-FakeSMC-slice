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

#define LogPrefix "ACPIMonitor: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super IOService
OSDefineMetaClassAndStructors(ACPIMonitor, IOService)

bool ACPIMonitor::addSensor(const char* method, const char* key, const char* type, unsigned char size)
{
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)key, (void *)type, (void *)size, (void *)this))
		return sensors->setObject(OSString::withCString(key), OSString::withCString(method));
	
	return 0;
}

bool ACPIMonitor::addTachometer(const char* method, const char* caption)
{
	for (int i = 0; i < 0x10; i++) {
		
		char name[5];
		
		snprintf(name, 5, KEY_FORMAT_FAN_SPEED, i); 
		
		if (addSensor(method, name, TYPE_FPE2, 2)) {
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
			
			return true;
		}
	}
	
	return false;
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
	
	acpiDevice = (IOACPIPlatformDevice *)provider;
	
	char key[5];
	
	//Here is Fan in ACPI	
	OSArray* fanNames = OSDynamicCast(OSArray, getProperty("Fan Names"));
	
	for (int i=0; i<10; i++) 
	{
		snprintf(key, 5, "FSN%X", i);
		
		if (kIOReturnSuccess == acpiDevice->validateObject(key)){
			OSString* name = NULL;
			
			if (fanNames )
				name = OSDynamicCast(OSString, fanNames->getObject(i));
			
			if (addTachometer(key, name ? name->getCStringNoCopy() : 0)) {
				WarningLog("Can't add tachometer sensor, kext will not load");
				return false;
			}
		} 
		else {
			break;
		}
	}

	//Next step - temperature keys
	if (kIOReturnSuccess == acpiDevice->validateObject("TSN0")){		
		addSensor("TSN0", "TC0H", TYPE_SP78, 2);
		InfoLog("%s registered", "TC0H");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("TSN1")){		
		addSensor("TSN1", "TN0H", TYPE_SP78, 2);
		InfoLog("%s registered", "TN0H");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("TSN2")){		
		addSensor("TSN2", "TW0P", TYPE_SP78, 2);
		InfoLog("%s registered", "TW0P");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("TSN3")){		
		addSensor("TSN3", "Th1H", TYPE_SP78, 2);
		InfoLog("%s registered", KEY_CPU_HEATSINK_TEMPERATURE);
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("TSN4")){		
		addSensor("TSN4", "Th1H", TYPE_SP78, 2);
		InfoLog("%s registered", "Th1H");
	}
	
	//Voltage
	if (kIOReturnSuccess == acpiDevice->validateObject("VSN0")){		
		addSensor("VSN0", "VCAC", TYPE_FP2E, 2);
		InfoLog("%s registered", "VCAC");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("VSN1")){		
		addSensor("VSN1", "Vp0C", TYPE_FP2E, 2);
		InfoLog("%s registered", "Vp0C");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("VSN2")){		
		addSensor("VSN2", "Vp1C", TYPE_FP2E, 2);
		InfoLog("%s registered", "Vp1C");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("VSN3")){		
		addSensor("VSN3", "Vp2C", TYPE_FP2E, 2);
		InfoLog("%s registered", "Vp2C");
	}
	
	//Amperage
	if (kIOReturnSuccess == acpiDevice->validateObject("ISN0")){		
		addSensor("ISN0", "ICAC", TYPE_UI16, 2);
		InfoLog("%s registered", "ICAC");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("ISN1")){		
		addSensor("ISN1", "Ip0C", TYPE_UI16, 2);
		InfoLog("%s registered", "Ip0C");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("ISN2")){		
		addSensor("ISN2", "Ip1C", TYPE_UI16, 2);
		InfoLog("%s registered", "Ip1C");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("ISN3")){		
		addSensor("ISN3", "Ip2C", TYPE_UI16, 2);
		InfoLog("%s registered", "Ip2C");
	}
	
	//Power
	if (kIOReturnSuccess == acpiDevice->validateObject("PSN0")){		
		addSensor("PSN0", "PC0C", TYPE_UI16, 2);
		InfoLog("%s registered", "PC0C");
	}
	if (kIOReturnSuccess == acpiDevice->validateObject("PSN1")){		
		addSensor("PSN1", "PC1C", TYPE_UI16, 2);
		InfoLog("%s registered", "PC1C");
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
	
	return true;
}

void ACPIMonitor::stop (IOService* provider)
{
	sensors->flushCollection();
	
	super::stop(provider);
}

void ACPIMonitor::free ()
{
	sensors->release();
	
	super::free();
}

inline UInt16 swap_value(UInt16 value)
{
	return ((value & 0xff00) >> 8) | ((value & 0xff) << 8);
}

inline UInt16 encode_fp2e(UInt16 value)
{
	UInt16 dec = (float)value / 1000.0f;
	UInt16 frc = value - (dec * 1000);
	
	return swap_value((dec << 14) | (frc << 4) /*| 0x3*/);
}

inline UInt16 encode_fpe2(UInt16 value)
{
	return swap_value(value << 2);
}

IOReturn ACPIMonitor::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void* data = param2;
		
		if (name && data) {
			if (OSString* key = OSDynamicCast(OSString, sensors->getObject(name))) {
#if __LP64__
				UInt64 value;
#else
				UInt32 value;
#endif
				
				if (kIOReturnSuccess == acpiDevice->evaluateInteger(key->getCStringNoCopy(), &value)) {
				
					UInt16 val = 0;
					
					if (key->getChar(0) == 'V') {
						val = encode_fp2e(value);
					}
					else if (key->getChar(0) == 'F') {
						val = encode_fpe2(value);
					}
					else val = value;
					
					bcopy(&val, data, 2);
					
					return kIOReturnSuccess;
				}
			}
			
			return kIOReturnBadArgument;
		}
		
		//DebugLog("bad argument key name or data");
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
