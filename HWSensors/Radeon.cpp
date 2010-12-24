/*
 *  Radeon.cpp
 *  HWSensors
 *
 *  Created by Sergey on 20.12.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "Radeon.h"


#define kGenericPCIDevice "IOPCIDevice"
#define kTimeoutMSecs 1000
#define fVendor "vendor-id"
#define fATYVendor "ATY,VendorID"
#define fDevice "device-id"
#define fClass	"class-code"
#define kIOPCIConfigBaseAddress0 0x10

#define INVID8(offset)		(mmio_base[offset])
#define INVID16(offset)		OSReadLittleInt16((mmio_base), offset)
#define INVID(offset)		OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val)	OSWriteLittleInt32((mmio_base), offset, val)

#define super IOService
OSDefineMetaClassAndStructors(RadeonMonitor, IOService)

bool RadeonMonitor::addSensor(const char* key, const char* type, unsigned char size, int index)
{
	if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)key, (void *)type, (void *)size, (void *)this))
		return sensors->setObject(key, OSNumber::withNumber(index, 32));	
	return false;
}

IOService* RadeonMonitor::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	bool ret = 0;
	if (OSDictionary * dictionary = serviceMatching(kGenericPCIDevice)) {
		if (OSIterator * iterator = getMatchingServices(dictionary)) {
			
			IOPCIDevice* device = 0;
			
			while (device = OSDynamicCast(IOPCIDevice, iterator->getNextObject())) {
				OSData *data = OSDynamicCast(OSData, device->getProperty(fVendor));
				if (data)
					vendor_id = *(UInt32*)data->getBytesNoCopy();
				else {
					data = OSDynamicCast(OSData, device->getProperty(fATYVendor));
					if (data)
						vendor_id = *(UInt32*)data->getBytesNoCopy();
				}
				
				data = OSDynamicCast(OSData, device->getProperty(fDevice));				
				if (data)
					device_id = *(UInt32*)data->getBytesNoCopy();
				
				data = OSDynamicCast(OSData, device->getProperty(fClass));				
				if (data)
					class_id = *(UInt32*)data->getBytesNoCopy();
				
				if ((vendor_id==0x1002) && (class_id = 0x03000000)) {
					InfoLog("found %lx Radeon chip", (long unsigned int)device_id);
					VCard = device;
					ret = 1; //TODO - count a number of cards
					break;
				}
				else {
					WarningLog("ATI Radeon not found!");
				}
			}
		}
	}
	if(ret)
		return this;
	else return 0;
}

bool RadeonMonitor::start(IOService * provider)
{
	if (!provider || !super::start(provider)) return false;
	
	if (!(fakeSMC = waitForService(serviceMatching(kFakeSMCService)))) {
		WarningLog("Can't locate fake SMC device, kext will not load");
		return false;
	}
	Card = new ATICard();
	Card->VCard = VCard;
	Card->chipID = device_id;	
	if(Card->initialize())	
	{
		char name[5];
		//try to find empty key
		for (int i = 0; i < 0x10; i++) {
			
			snprintf(name, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, i); 
			
			UInt8 length = 0;
			void * data = 0;
			
			IOReturn result = fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)name, (void *)&length, (void *)&data, 0);
			
			if (kIOReturnSuccess == result) {
				continue;
			}
			if (addSensor(name, TYPE_SP78, 2, i)) {
				numCard = i;
				break;
			}
		}
		
		if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)name, (void *)TYPE_SP78, (void *)2, this)) {
			WarningLog("Can't add key to fake SMC device, kext will not load");
			return false;
		}
		
		return true;	
	}
	else {
		return false;
	}
}


bool RadeonMonitor::init(OSDictionary *properties)
{
    if (!super::init(properties))
		return false;
	
	if (!(sensors = OSDictionary::withCapacity(0)))
		return false;
	
	return true;
}

void RadeonMonitor::stop (IOService* provider)
{
	sensors->flushCollection();
	
	super::stop(provider);
}

void RadeonMonitor::free ()
{
	sensors->release();
	
	super::free();
}

IOReturn RadeonMonitor::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	UInt16 t;
	
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void* data = param2;
		
		if (name && data) {
			if (OSNumber *number = OSDynamicCast(OSNumber, sensors->getObject(name))) {				
				UInt32 index = number->unsigned16BitValue();
				if (index != numCard) {  //TODO - multiple card support
					return kIOReturnBadArgument;
				}
			}
			
			switch (Card->tempFamily) {
				case R6xx:
					Card->R6xxTemperatureSensor(&t);
					break;
				case R7xx:
					Card->R7xxTemperatureSensor(&t);
					break;
				case R8xx:
					Card->EverTemperatureSensor(&t);
					break;
				default:
					break;
			}
			//t = Card->tempSensor->readTemp(index);
			bcopy(&t, data, 2);
			
			return kIOReturnSuccess;
		}
		
		//DebugLog("bad argument key name or data");
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
