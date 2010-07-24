/* 
 *  FakeSMC ATI Radeon GPU monitoring plugin
 *  Created by Slice 23.07.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCRadeonMon.h"

#define kTimeoutMSecs 1000
#define fVendor "vendor-id"
#define fDevice "device-id"

//#define kIOPCIConfigBaseAddress0 0x10

#undef super
#define super IOService

OSDefineMetaClassAndStructors(RadeonPlugin, IOService) 

//static int count=0;

IOService*
RadeonPlugin::probe(IOService *provider, SInt32 *score)
{
	
	OSData*		prop;
	
	prop = OSDynamicCast( OSData , provider->getProperty(fVendor)); // safe way to get vendor
	if(prop)
	{
		chipID = *(UInt32*) prop->getBytesNoCopy();
		IOLog("FakeSMC_Radeon: found %lx chip\n", (long unsigned int)chipID);
		if( (chipID & 0xffff) != 0x1002) //check if vendorID is really ATI, if not don't bother
		{
			//IOLog("FakeSMC_Radeon: Can't Find ATI Chip!\n");
			return( 0 );
		}		
	}
	
    if( !super::probe( provider, score ))
		return( 0 );
	//	IOLog("FakeSMC_Radeon: probe success\n");	
	return (this);
}

bool
RadeonPlugin::start( IOService * provider ) {
	if(!provider || !super::start(provider))
		return false;
	Card = new ATICard();
	Card->VCard = (IOPCIDevice*)provider;
	Card->chipID = chipID;	
	return Card->initialize();	
	
}

bool RadeonPlugin::init(OSDictionary *properties)
{    
	return super::init(properties);
}

void RadeonPlugin::stop (IOService* provider)
{
	if (Card) {
		delete Card;
	}
	UpdateFNum();
	super::stop(provider);
}

void RadeonPlugin::free ()
{
	super::free ();
}

