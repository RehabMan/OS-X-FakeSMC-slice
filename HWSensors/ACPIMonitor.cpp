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
		if (sensors->setObject(key, OSString::withCString(method))) {
			InfoLog("%s registered", method);
			return true;
		}
	
	return false;
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
			else {
				if (kIOReturnSuccess == result) {
					length = 0;
					
					bcopy(data, &length, 1);
					
					length++;
					
					if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, false, (void *)KEY_FAN_NUMBER, (void *)1, (void *)&length, 0))
						WarningLog("error updating FNum value");
				}
				else WarningLog("error reading FNum value");
			}
			
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
		snprintf(key, 5, "FAN%X", i);
		
		if (kIOReturnSuccess == acpiDevice->validateObject(key)){
			OSString* name = NULL;
			
			if (fanNames )
				name = OSDynamicCast(OSString, fanNames->getObject(i));
			
			if (!addTachometer(key, name ? name->getCStringNoCopy() : 0))
				WarningLog("Can't add tachometer sensor, key %s", key);
		} 
		else {
			snprintf(key, 5, "FTN%X", i);
			if (kIOReturnSuccess == acpiDevice->validateObject(key)){
				OSString* name = NULL;
				
				if (fanNames )
					name = OSDynamicCast(OSString, fanNames->getObject(i));
				
				if (!addTachometer(key, name ? name->getCStringNoCopy() : 0))
					WarningLog("Can't add tachometer sensor, key %s", key);
			} 
			else
			break;
		}
	}

	//Next step - temperature keys
	if (kIOReturnSuccess == acpiDevice->validateObject("TCPU"))
		addSensor("TCPU", KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("TSYS"))
		addSensor("TSYS", KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("TDIM"))
		addSensor("TDIM", KEY_DIMM_TEMPERATURE, TYPE_SP78, 2);
		
	if (kIOReturnSuccess == acpiDevice->validateObject("TAMB"))
		addSensor("TAMB", KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2);
	
    if (kIOReturnSuccess == acpiDevice->validateObject("TCPP"))
        addSensor("TCPP", KEY_CPU_PROXIMITY_TEMPERATURE, TYPE_SP78, 2);
	// We should add also GPU reading stuff for those who has no supported plug in but have the value on EC registers
	
	
	
	//Voltage
	if (kIOReturnSuccess == acpiDevice->validateObject("VCPU"))
		addSensor("VSN0", KEY_CPU_VOLTAGE, TYPE_FP2E, 2);
	
	if (kIOReturnSuccess == acpiDevice->validateObject("VMEM"))
		addSensor("VSN0", KEY_MEMORY_VOLTAGE, TYPE_FP2E, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("VSN1"))
		addSensor("VSN1", "Vp0C", TYPE_FP2E, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("VSN2"))
		addSensor("VSN2", "Vp1C", TYPE_FP2E, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("VSN3"))
		addSensor("VSN3", "Vp2C", TYPE_FP2E, 2);
	
	//Amperage
	if (kIOReturnSuccess == acpiDevice->validateObject("ISN0"))
		addSensor("ISN0", "ICAC", TYPE_UI16, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("ISN1"))
		addSensor("ISN1", "Ip0C", TYPE_UI16, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("ISN2"))
		addSensor("ISN2", "Ip1C", TYPE_UI16, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("ISN3"))
		addSensor("ISN3", "Ip2C", TYPE_UI16, 2);
	
	//Power
	if (kIOReturnSuccess == acpiDevice->validateObject("PSN0"))
		addSensor("PSN0", "PC0C", TYPE_UI16, 2);

	if (kIOReturnSuccess == acpiDevice->validateObject("PSN1"))
		addSensor("PSN1", "PC1C", TYPE_UI16, 2);
	
	// AC Power/Battery
    if (kIOReturnSuccess == acpiDevice->validateObject("ACDC")) // Power Source Read AC/Battery
	{ 
		addSensor("ACDC", "ACEN", TYPE_UI8, 1);
		addSensor("ACDC", "ACFP", TYPE_FLAG, 1);
		addSensor("ACDC", "ACIN", TYPE_FLAG, 1);
	}
	// TODO real SMC returns ACID only when AC is plugged, if not is zeroed, so hardcoding it in plist is not OK IMHO
	// Same goes for ACIC, but no idea how we can get the AC current value..
	
	// Here if ACDC returns 0 we need to set the on battery BATP flag
	
	// Battery stuff, need to implement rest of the keys once i figure those
    if (kIOReturnSuccess == acpiDevice->validateObject("BAK0")) // Battery 0 Current
        addSensor("BAK0", "B0AC", TYPE_SI16, 2);
	
    if (kIOReturnSuccess == acpiDevice->validateObject("BAK1")) // Battery 0 Voltage
        addSensor("BAK1", "B0AV", TYPE_UI16, 2);
	
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
#define MEGA10 10000000ull
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
						if (key->getChar(1) == 'A') {
							val = encode_fpe2(value);
						} else 
							if (key->getChar(1) == 'T') {
								val = encode_fpe2(MEGA10 / value);
							}
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
