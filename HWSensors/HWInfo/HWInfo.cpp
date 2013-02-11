/*
 *  HWInfo.cpp
 *  HWSensors
 *
 *  Created by Slice on 11.02.13.
 *  
 *
 */

#include "HWInfo.h"
#include "../FakeSMC.h"

#define Debug TRUE

#define LogPrefix "HWInfo: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super IOService
OSDefineMetaClassAndStructors(HWInfo, IOService)

bool HWInfo::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
  if (!super::init(properties))
		return false;
  
	return true;
}

IOService* HWInfo::probe(IOService *provider, SInt32 *score)
{
  //the purpose of probe() is to check the hardware and find device to monitor
	
	if (super::probe(provider, score) != this) return 0;
	
	InfoLog("HWInfo get information from Clover bootloader, (c)Slice 2013");
	
	return this;
}

bool HWInfo::start(IOService * provider)
{
  IORegistryEntry * rootNode;
  OSData *data;
  UInt8 length = 0;
	
	if (!super::start(provider)) return false;
	DebugLog("Starting...");
  
  if (!(fakeSMC = waitForService(serviceMatching(kFakeSMCDeviceService)))) {
		WarningLog("Can't locate fake SMC device, kext will not load");
		return false;
	}
  //first we get all values from DT	
  rootNode = fromPath("/efi/platform", gIODTPlane);
  if (rootNode) {   
    
    data = OSDynamicCast(OSData, rootNode->getProperty("RPlt"));
    if (data) {
      //      bzero(Platform, 8);
      bcopy(data->getBytesNoCopy(), Platform, 8);
      InfoLog("SMC Platform: %s", Platform);
    }
    
    data = OSDynamicCast(OSData, rootNode->getProperty("RBr"));
    if (data) {
      bcopy(data->getBytesNoCopy(), PlatformB, 8);
      InfoLog("SMC Branch: %s", PlatformB);
    }
    
    data = OSDynamicCast(OSData, rootNode->getProperty("REV"));
    if (data) {
      bcopy(data->getBytesNoCopy(), SMCRevision, 6);
      InfoLog("SMC Revision set to: %01x.%02xf%02x", SMCRevision[0], SMCRevision[1], SMCRevision[5]);
    }
    
    data = OSDynamicCast(OSData, rootNode->getProperty("EPCI"));
    if (data) {
      SMCConfig = *(UInt32*)data->getBytesNoCopy();
      InfoLog("SMC ConfigID set to: %02x %02x %02x %02x",
              (unsigned int)SMCConfig & 0xFF,
              (unsigned int)(SMCConfig >> 8) & 0xFF, 
              (unsigned int)(SMCConfig >> 16) & 0xFF,
              (unsigned int)(SMCConfig >> 24) & 0xFF);
    }
    
    data = OSDynamicCast(OSData, rootNode->getProperty("BEMB"));
    if (data) {
      Mobile = *(UInt8*)data->getBytesNoCopy();
    }
    
  } 
  //then we add or renew keys  
  if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)"RPlt", (void *)&length, (void *)&data, 0)) {
    //if success means the key is exists and we should replace the value
    if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void *)"RPlt", (void *)8, (void *)&Platform, 0)) {
      WarningLog("error updating RPlt value");
    }
  } else {
    //create a key up to caller
    if (Platform[0] != 'N') {
      if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)"RPlt", (void *)"ch8*", (void *)8, this)) {
        WarningLog("Can't add Platform key to fake SMC device");
      }			
    }    
  }
  
  if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)"RBr", (void *)&length, (void *)&data, 0)) {
    //if success means the key is exists and we should replace the value
    if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void *)"RBr", (void *)8, (void *)&PlatformB, 0)) {
      WarningLog("error updating RBr value");
    }
  } else {
    //create a key up to caller
    if (Platform[0] != 'N') {
      if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)"RBr", (void *)"ch8*", (void *)8, this)) {
        WarningLog("Can't add Branch key to fake SMC device");
      }			
    }    
  }
  
  if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)"REV", (void *)&length, (void *)&data, 0)) {
    //if success means the key is exists and we should replace the value
    if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void *)"REV", (void *)6, (void *)&SMCRevision, 0)) {
      WarningLog("error updating RBr value");
    }
  } else {
    //create a key up to caller
    if (Platform[0] != 'N') {
      if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)"REV", (void *)"ch8*", (void *)6, this)) {
        WarningLog("Can't add Branch key to fake SMC device");
      }			
    }    
  }
  
  if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)"EPCI", (void *)&length, (void *)&data, 0)) {
    //if success means the key is exists and we should replace the value
    if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void *)"EPCI", (void *)4, (void *)&SMCConfig, 0)) {
      WarningLog("error updating EPCI value");
    }
  } else {
    //create a key up to caller
    if (Platform[0] != 'N') {
      if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)"EPCI", (void *)"ch8*", (void *)4, this)) {
        WarningLog("Can't add config key to fake SMC device");
      }			
    }    
  }
  
  if (kIOReturnSuccess == fakeSMC->callPlatformFunction(kFakeSMCGetKeyValue, true, (void *)"BEMB", (void *)&length, (void *)&data, 0)) {
    //if success means the key is exists and we should replace the value
    if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCSetKeyValue, true, (void *)"BEMB", (void *)1, (void *)&Mobile, 0)) {
      WarningLog("error updating EPCI value");
    }
  } else {
    //create a key up to caller
    if (Platform[0] != 'N') {
      if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)"BEMB", (void *)"ch8*", (void *)1, this)) {
        WarningLog("Can't add config key to fake SMC device");
      }			
    }    
  }
	return true;
}

void HWInfo::stop (IOService* provider)
{
	DebugLog("Stoping...");
	super::stop(provider);
}

void HWInfo::free ()
{
	DebugLog("Freeing...");
	
	super::free ();
}

IOReturn HWInfo::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void * data = param2;
		//UInt32 size = (UInt64)param3;
//		UInt16 value=0;
		//InfoLog("key %s is called", name);
		if (name && data) {
			
			switch (name[0]) {
				case 'E':
					if ((name[1] == 'P') || (name[2] == 'C') || (name[3] == 'I')) {
            *(UInt32*)data = SMCConfig;
            return kIOReturnSuccess;
          }
					break;
        case 'B':
					if ((name[1] == 'E') || (name[2] == 'M') || (name[3] == 'B')) {
            *(UInt8*)data = Mobile;
            return kIOReturnSuccess;
          }
          break;
				case 'R':
					if ((name[1] == 'P') || (name[2] == 'l') || (name[3] == 't')) {
						bcopy(Platform, data, 8);
            return kIOReturnSuccess;
          }
					if ((name[1] == 'B') || (name[2] == 'r')) {
						bcopy(PlatformB, data, 8);
            return kIOReturnSuccess;
          }
					if ((name[1] == 'E') || (name[2] == 'V')) {
						bcopy(SMCRevision, data, 6);
            return kIOReturnSuccess;
          }
          
				default:
					return kIOReturnBadArgument;
			}
      
      //			bcopy(&value, data, 2);		
      //			return kIOReturnSuccess;
		}
		
		//DebugLog("bad argument key name or data");
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}
