/*
 *  SuperIOFamily.cpp
 *  HWSensors
 *
 *  Created by mozo on 08/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include <architecture/i386/pio.h>
#include <libkern/c++/OSCollection.h>

#include "SuperIOFamily.h"
#include "FakeSMC.h"

#define Debug FALSE

#define LogPrefix "SuperIOFamily: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

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

// Sensor

OSDefineMetaClassAndStructors(SuperIOSensor, OSObject)

SuperIOSensor *SuperIOSensor::withOwner(SuperIOMonitor *aOwner, const char* aKey, const char* aType, unsigned char aSize, SuperIOSensorGroup aGroup, unsigned long aIndex)
{
	SuperIOSensor *me = new SuperIOSensor;
	
    if (me && !me->initWithOwner(aOwner, aKey, aType, aSize, aGroup, aIndex)) {
        me->release();
        return 0;
    }
	
    return me;
}

const char *SuperIOSensor::getName()
{
	return name;
}

const char *SuperIOSensor::getType()
{
	return type;
}

unsigned char SuperIOSensor::getSize()
{
	return size;
}

SuperIOSensorGroup SuperIOSensor::getGroup()
{
	return group;
}

unsigned long SuperIOSensor::getIndex()
{
	return index;
}

long SuperIOSensor::getValue()
{
	UInt16 value = 0;
	
	switch (group) {
		case kSuperIOTemperatureSensor:
			value = owner->readTemperature(index);
			break;
		case kSuperIOVoltageSensor:
			value = owner->readVoltage(index);
			break;
		case kSuperIOTachometerSensor:
			value = owner->readTachometer(index);
			break;
		default:
			break;
	}
	
	if (*((uint32_t*)type) == *((uint32_t*)TYPE_FP2E)) {
		value = encode_fp2e(value);
	}
	else if (*((uint32_t*)type) == *((uint32_t*)TYPE_FPE2)) {
		value = encode_fpe2(value);
	}
	
	return value;
}

bool SuperIOSensor::initWithOwner(SuperIOMonitor *aOwner, const char* aKey, const char* aType, unsigned char aSize, SuperIOSensorGroup aGroup, unsigned long aIndex)
{
	if (!OSObject::init())
		return false;
	
	if (!(owner = aOwner))
		return false;
	
	if (!(name = (char *)IOMalloc(5)))
		return false;
	
	bcopy(aKey, name, 4);
	name[5] = '\0';
	
	if (!(type = (char *)IOMalloc(5)))
		return false;
	
	bcopy(aType, type, 4);
	type[5] = '\0';
	
	size = aSize;
	group = aGroup;
	index = aIndex;
	
	return true;
}

void SuperIOSensor::free()
{
	if (name)
		IOFree(name, 5);
	
	if (type)
		IOFree(type, 5);
	
	OSObject::free();
}

// Monitor

#define super IOService
OSDefineMetaClassAndAbstractStructors(SuperIOMonitor, IOService)

UInt8 SuperIOMonitor::listenPortByte(UInt8 reg)
{
	outb(registerPort, reg);
	return inb(valuePort);
}

UInt16 SuperIOMonitor::listenPortWord(UInt8 reg)
{
	return ((listenPortByte(reg) << 8) | listenPortByte(reg + 1));
}

void SuperIOMonitor::selectLogicalDevice(UInt8 num)
{
	outb(registerPort, SUPERIO_DEVICE_SELECT_REGISTER);
	outb(valuePort, num);
}

bool SuperIOMonitor::getLogicalDeviceAddress(UInt8 reg)
{
	address = listenPortWord(reg);
	
	if (address < 0x100 || (address & 0xF007) != 0)
		return false;
	
	IOSleep(250);
	
	if (address != listenPortWord(reg))
		return false;
	
	return true;
}

int SuperIOMonitor::getPortsCount() 
{ 
	return 2; 
};

void SuperIOMonitor::selectPort(unsigned char index)
{
	registerPort = SUPERIO_STANDART_PORT[index]; 
	valuePort = SUPERIO_STANDART_PORT[index] + 1;
}

bool SuperIOMonitor::probePort() 
{		
	return true; 
};

long SuperIOMonitor::readVoltage(unsigned long index)
{
	return 0;
}

long SuperIOMonitor::readTachometer(unsigned long index)
{
	return 0;
}

long SuperIOMonitor::readTemperature(unsigned long index)
{
	return 0;
}

void SuperIOMonitor::enter()
{
	//
};

void SuperIOMonitor::exit()
{
	//
};

bool SuperIOMonitor::updateSensor(const char *key, const char *type, unsigned char size, SuperIOSensorGroup group, unsigned long index)
{
	long value = 0;
	
	switch (group) {
		case kSuperIOTemperatureSensor:
			value = readTemperature(index);
			break;
		case kSuperIOVoltageSensor:
			value = readVoltage(index);
			break;
		case kSuperIOTachometerSensor:
			value = readTachometer(index);
			break;
		default:
			break;
	}

	if (strcmp(type, TYPE_FP2E) == 0) {
		value = encode_fp2e(value);
	}
	else if (strcmp(type, TYPE_FPE2) == 0) {
		value = encode_fpe2(value);
	}
	
	if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void*)key, (void*)size, (void*)&value, 0))
		return false;
	
	return true;
}

const char *SuperIOMonitor::getModelName()
{
	return "Unknown";
}

SuperIOSensor *SuperIOMonitor::addSensor(const char* name, const char* type, unsigned char size, SuperIOSensorGroup group, unsigned long index)
{
	if (NULL != getSensor(name))
		return 0;
	
	if (SuperIOSensor *sensor = SuperIOSensor::withOwner(this, name, type, size, group, index))
		if (sensors->setObject(sensor))
			if(kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)name, (void *)type, (void *)size, (void *)this))
				return sensor;
	
	return 0;
}

SuperIOSensor *SuperIOMonitor::addTachometer(unsigned long index, const char* id)
{
	UInt8 length = 0;
	void * data = 0;
	
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)KEY_FAN_NUMBER, (void *)&length, (void *)&data, 0)) {
		length = 0;
		
		bcopy(data, &length, 1);
		
		char name[5];
		
		snprintf(name, 5, KEY_FORMAT_FAN_SPEED, length); 
		
		if (SuperIOSensor *sensor = addSensor(name, TYPE_FPE2, 2, kSuperIOTachometerSensor, index)) {
			if (id) {
				snprintf(name, 5, KEY_FORMAT_FAN_ID, length); 
				
				if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyValue, false, (void *)name, (void *)TYPE_CH8, (void *)((UInt64)strlen(id)), (void *)id))
					WarningLog("error adding tachometer id value");
			}
			
			length++;
			
			if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void *)KEY_FAN_NUMBER, (void *)1, (void *)&length, 0))
				WarningLog("error updating FNum value");
			
			return sensor;
		}
	}
	else WarningLog("error reading FNum value");
	
	return 0;
}
		
SuperIOSensor *	SuperIOMonitor::getSensor(const char* key) 
{
	if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(sensors)) {
		
		//UInt32 key1 = *((uint32_t*)key);
		UInt32 key1 = *((uint32_t*)key);
		
		
		while (SuperIOSensor *sensor = OSDynamicCast(SuperIOSensor, iterator->getNextObject())) {
			UInt32 key2 = *((uint32_t*)sensor->getName());
			
			if (key1 == key2)
				return sensor;
		}
	}
	
	return 0;
}

bool SuperIOMonitor::init(OSDictionary *properties)
{
	DebugLog("initialising...");
	
	if (!super::init(properties))
		return false;

	if (!(sensors = OSArray::withCapacity(0)))
		return false;
	
	model = 0;

	return true;
}

IOService *SuperIOMonitor::probe(IOService *provider, SInt32 *score)
{
	DebugLog("probing...");
	
	if (super::probe(provider, score) != this) 
		return 0;
	
	for (UInt8 i = 0; i < getPortsCount(); i++)
	{	
		selectPort(i);
		
		enter();
		
		if (probePort()) {
			exit();
			return this;
		}
		
		exit();
	}
	
	return 0;
}

bool SuperIOMonitor::start(IOService *provider)
{		
	DebugLog("starting...");
	
	if (!super::start(provider)) return false;
	
	if (!(fakeSMC = waitForService(serviceMatching(kFakeSMCService)))) {
		WarningLog("can't locate fake SMC device, kext will not load");
		return false;
	}
	
	return true;
}

void SuperIOMonitor::stop(IOService* provider)
{
	DebugLog("stoping...");
	
	super::stop(provider);
}

void SuperIOMonitor::free()
{
	DebugLog("freeing...");
	
	sensors->release();
	
	super::free();
}

IOReturn SuperIOMonitor::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void * data = param2;
		//UInt32 size = (UInt64)param3;
		
		if (name && data)
			if (SuperIOSensor *sensor = getSensor(name)) {
				UInt16 value = sensor->getValue();
				
				bcopy(&value, data, 2);
				
				return kIOReturnSuccess;
			}
		
		return kIOReturnBadArgument;
	}

	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}