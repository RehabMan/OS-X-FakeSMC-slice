#include "fakesmc.h"
#include "FakeSMCBinding.h"

#define SMC_DATA_PORT 0x00
#define SMC_CMD_PORT 0x04

IOACPIPlatformDeviceCh * smcNode;

void FakeSMCAddKey (const char* keyname, uint8_t keylen, char* keydata)
{
	smcNode->SMCAddKey(keyname, keylen, keydata, 1);
	smcNode->FixUpKeysNum();
}

void FakeSMCAddKey (const char* keyname, const char* keytype, uint8_t keylen, char* keydata)
{
	smcNode->SMCAddKey(keyname, keytype, keylen, keydata, 1);
	smcNode->FixUpKeysNum();
}

void FakeSMCAddKey(const char* keyname, uint8_t keylen, char* keydata, FakeSMCBinding* binding)
{
	smcNode->SMCAddKey(keyname, keylen, keydata, 1, binding);
	smcNode->FixUpKeysNum();
}

void FakeSMCAddKey (const char* keyname, const char* keytype, uint8_t keylen, char* keydata, FakeSMCBinding* binding)
{
	smcNode->SMCAddKey(keyname, keytype, keylen, keydata, 1, binding);
	smcNode->FixUpKeysNum();
}

char* FakeSMCReadKey (const char* keyname)
{
	if(SMCData node = smcNode->FindSMCKey(keyname)) 
		return node->data;
		
	return NULL;
}

SMCData FakeSMCGetKey (const char* keyname)
{
	return smcNode->FindSMCKey(keyname);
}

void FakeSMCRemoveKeyBinding (const char* keyname)
{
	if(SMCData node = smcNode->FindSMCKey(keyname))
	{
		node->binding = NULL;
	}
}

#define super IOService
OSDefineMetaClassAndStructors (FakeSMC, IOService)

bool FakeSMC::init (OSDictionary *dict)
{
    bool res = super::init (dict);
	
    return res;
}

bool FakeSMC::start (IOService *provider)
{
	IOLog ("FakeSMC: Opensource SMC device emulator by netkas (C) 2009\nFakeSMC: Monitoring plugins support by mozodojo (C) 2010\nFakeSMC: Original idea of plugins and code sample by usr-sse2 (C) 2010\n");
	bool res = super::start (provider);
	bool ret=false;
	OSArray *        controllers;
    OSArray *        specifiers;
	OSArray *	array;
	OSData *          tmpData    = 0;
	OSData *          tmpDatak    = 0;
	OSString *tmpString = 0;
	UInt64 line = 0x06;
	OSSymbol * gIntelPICName;
	const char * nodeName = "APP0001";
	const char * nodeComp = "smc-napa";
	OSString *smccomp = 0;
	OSDictionary *keysToAdd = 0;
	OSIterator *iter = 0;
	const OSSymbol *dictKey = 0;
	OSBoolean * debugkey = 0;
	bool debug = true;

	
	smc_array = IOMalloc(0x20);
	bzero(smc_array,0x20);
	IODeviceMemory::InitElement	rangeList[1];
	smcNode = new IOACPIPlatformDeviceCh;
	if (!smcNode) {
		IOLog("failed to create smcnode\n");
		return false;
	}
	if (!smcNode->init(provider,0,0)) {
		IOLog("failed to init smcnode\n");
		return false;
	}
	smcNode->setName("SMC");
	smcNode->setProperty("name",(void *)nodeName, strlen(nodeName)+1);
	smccomp = OSDynamicCast(OSString, getProperty("smc-compatible"));
	if(smccomp) {
		smcNode->setProperty("compatible",(void *)smccomp->getCStringNoCopy(), smccomp->getLength()+1);
	} else {
		smcNode->setProperty("compatible",(void *)nodeComp, strlen(nodeComp)+1);
	}
	smcNode->setProperty("_STA", (unsigned long long)0x0000000b, 32);
	smcNode->SMCSetup();
	///adding keys from plist
	smcNode->SMCAddKey("CIST", 4, (char*)"IBST", 1);
	keysToAdd = OSDynamicCast(OSDictionary, getProperty("SMCKeys"));
	if (keysToAdd){
		iter = OSCollectionIterator::withCollection(keysToAdd);
		if (iter) {
			while ((dictKey = (const OSSymbol *)iter->getNextObject())) {
				
				if(dictKey->getLength() != 4) {
					IOLog("FakeSMC: key %s has wrong length %d, should be 4, not adding\n", dictKey->getCStringNoCopy(), dictKey->getLength());
					continue;
				}
				tmpString = OSDynamicCast(OSString, keysToAdd->getObject(dictKey));
				if (tmpString) {
					if(tmpString->getLength() > 255) {
						IOLog("FakeSMC: key %s is too big( %d bytes)\n", dictKey->getCStringNoCopy(), tmpString->getLength());
						continue;
					}
					smcNode->SMCAddKey(dictKey->getCStringNoCopy(), tmpString->getLength(), (char* )tmpString->getCStringNoCopy(), 1);
				}
				
				tmpDatak = OSDynamicCast(OSData, keysToAdd->getObject(dictKey));
				if (tmpDatak) {
					if(tmpDatak->getLength() > 255) {
						IOLog("FakeSMC: key %s is too big( %d bytes)\n", dictKey->getCStringNoCopy(), tmpDatak->getLength());
						continue;
					}
					smcNode->SMCAddKey(dictKey->getCStringNoCopy(), tmpDatak->getLength(), (char* )tmpDatak->getBytesNoCopy(), 1);
				}
			}
		}
	}
	
	debugkey = OSDynamicCast(OSBoolean, getProperty("debug"));
	
	if(debugkey)
		debug=debugkey->getValue();
		
	smcNode->SetDebug(debug);
	smcNode->FixUpKeysNum();
	rangeList[0].start = 0x300;
	rangeList[0].length = 0x20;
	
	array = IODeviceMemory::arrayFromList(rangeList, 1);
	
	if(!array) {
		IOLog("failed to create Device memory array\n");
		return false;
	}
	
	smcNode->setDeviceMemory(array);
	array->release();
	
	
	controllers = OSArray::withCapacity(1);
	if(!controllers) {
		IOLog("failed to create controllers array\n");
	}
	
	specifiers  = OSArray::withCapacity(1);
	if(!specifiers) {
		IOLog("failed to create specifiers array\n");
	}
	
	tmpData = OSData::withBytes( &line, sizeof(line) );
	if ( !tmpData ) {
		IOLog("failed to create tmpdata\n");
	}
	gIntelPICName = (OSSymbol *) OSSymbol::withCStringNoCopy("io-apic-0");
	specifiers->setObject( tmpData );
	controllers->setObject( gIntelPICName );
	ret = smcNode->setProperty( gIOInterruptControllersKey, controllers ) && smcNode->setProperty( gIOInterruptSpecifiersKey,  specifiers );
	
	smcNode->attachToParent(provider, gIOServicePlane);
	smcNode->registerService();
	SMCDevice = smcNode;
	
	return res;
}

IOService * FakeSMC::probe(IOService *provider, SInt32 *score)
{
	IOService * ret = super::probe(provider, score);
    return ret;
}

void FakeSMC::stop (IOService *provider)
{
    super::stop (provider);
}

void FakeSMC::free ()
{
    super::free ();
}