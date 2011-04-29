#include "FakeSMC.h"

#define Debug FALSE

#define LogPrefix "FakeSMC: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super IOService
OSDefineMetaClassAndStructors (FakeSMC, IOService)

bool FakeSMC::init(OSDictionary *dictionary)
{	
	if (!super::init(dictionary))
		return false;
		
	return true;
}

IOService *FakeSMC::probe(IOService *provider, SInt32 *score)
{
    if (!super::probe(provider, score))
		return 0;
	
	InfoLog("opensource SMC device emulator by netkas (C) 2009");
	InfoLog("plugins & plugins support modifications by mozodojo, usr-sse2, slice (C) 2010");
	
	return this;
}

bool FakeSMC::start(IOService *provider)
{
	if (!super::start(provider)) return false;
			
	if (!(smcDevice = new FakeSMCDevice)) {
		InfoLog("failed to create smcDevice");
		return false;
	}
		
	if (!smcDevice->init(provider, OSDynamicCast(OSDictionary, getProperty("Configuration")))) {
		InfoLog("failed to init smcDevice");
		return false;
	}
	
	smcDevice->registerService();
	registerService();
		
	return true;
}

void FakeSMC::stop(IOService *provider)
{
    super::stop(provider);
}

void FakeSMC::free()
{
    super::free();
}

IOReturn FakeSMC::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	if (functionName->isEqualTo(kFakeSMCSetKeyValue)) {
		const char *name = (const char *)param1;
		unsigned char size = (UInt64)param2;
		const void *data = (const void *)param3;
		
		if (name && data && size > 0) {
						
			if (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, smcDevice->getKey(name)))
				if (key->setValueFromBuffer(data, size))
					return kIOReturnSuccess;
			
			return kIOReturnError;
		}
		
		return kIOReturnBadArgument;
	}
	else if (functionName->isEqualTo(kFakeSMCAddKeyHandler)) {
		const char *name = (const char *)param1;
		const char *type = (const char *)param2;
		unsigned char size = (UInt64)param3;
		IOService *handler = (IOService *)param4;
		
		if (name && type && size > 0) {
			
			DebugLog("adding key %s with handler, type %s, size %d", name, type, size);
			
			if (smcDevice->addKeyWithHandler(name, type, size, handler))
				return kIOReturnSuccess;

			return kIOReturnError;
		}
		
		return kIOReturnBadArgument;
	}
	else if (functionName->isEqualTo(kFakeSMCAddKeyValue)) {
		const char *name = (const char *)param1;
		const char *type = (const char *)param2;
		unsigned char size = (UInt64)param3;
		const void *value = (const void *)param4;
		
		if (name && type && size > 0) {
			
			DebugLog("adding key %s with value, type %s, size %d", name, type, size);
			
			if (smcDevice->addKeyWithValue(name, type, size, value))
				return kIOReturnSuccess;

			return kIOReturnError;
		}
		
		return kIOReturnBadArgument;
	}
	else if (functionName->isEqualTo(kFakeSMCGetKeyValue)) {
		const char *name = (const char *)param1;
		UInt8 *size = (UInt8*)param2;
		const void **value = (const void **)param3;
		
		if (name) {
			if (FakeSMCKey *key = smcDevice->getKey(name)) {
				*size = key->getSize();
				*value = key->getValue();
				
				return kIOReturnSuccess;
			}
		
			return kIOReturnError;
		}
		
		return kIOReturnBadArgument;
	}
	
	return kIOReturnUnsupported;
}