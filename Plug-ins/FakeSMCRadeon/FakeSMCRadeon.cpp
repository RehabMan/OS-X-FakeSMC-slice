/* 
 *  FakeSMC ATI Radeon GPU monitoring plugin
 *  Created by Slice 23.07.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCRadeon.h"

#define kTimeoutMSecs 1000
#define fVendor "vendor-id"
#define fDevice "device-id"

#define kIOPCIConfigBaseAddress0 0x10


#define INVID8(offset) (mmio_base[offset])
#define INVID16(offset) OSReadLittleInt16((mmio_base), offset)
#define INVID(offset) OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((mmio_base), offset, val)

OSDefineMetaClassAndStructors(RadeonPlugin, IOService) //IOPCIDevice)

RadeonPlugin * RadeonService;
//static int count=0;

#define super IOService
IOReturn RadeonPlugin::Update(const char* key, char* data) {	
	if(CompareKeys(key, "TG0D"))
	{
		short value;
			if (mmio_base) {
				/*
				OUTVID(TIC1, 3);
				//		if ((INVID16(TSC1) & (1<<15)) && !(INVID16(TSC1) & (1<<8)))//enabled and ready
				for (int i=0; i<1000; i++) {  //attempts to ready
					
					if (INVID16(TSS1) & (1<<10))   //valid?
						break;
					IOSleep(10);
				}				
				value = INVID8(TR1);
				 */
			}				
				
		data[0] = 140 - value;
		data[1] = 0;
		return kIOReturnSuccess;
	} 
	return kIOReturnUnsupported;
}

IOService*
RadeonPlugin::probe(IOService *provider, SInt32 *score)
{
	
	OSData*		prop;
	UInt32		Vchip;
	
	prop = OSDynamicCast( OSData , provider->getProperty(fVendor)); // safe way to get vendor
	if(prop)
	{
		Vchip = *(UInt32*) prop->getBytesNoCopy();
		IOLog("FakeSMC_Radeon: found %lx chip\n", (long unsigned int)Vchip);
		if( (Vchip & 0xffff) != 0x1002) //check if vendorID is really ATI, if not don't bother
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
	//	OSData*		idKey;	
	
	VCard = (IOPCIDevice*)provider;
	VCard->setMemoryEnable(true);
/*	
	for (int i=0; i<0xff; i +=16) {
		IOLog("%02lx: ", (long unsigned int)i);
		for (int j=0; j<16; j += 4) {
			IOLog("%08lx ", (long unsigned int)VCard->configRead32(i+j));
		}
		IOLog("\n");
	}
 */
	for (UInt32 i = 0; (mmio = VCard->mapDeviceMemoryWithIndex(i)); i++)
	{
		long unsigned int mmio_base_phys = mmio->getPhysicalAddress();
		// Make sure we  select MMIO registers
		if (((mmio->getLength()) == 0x00010000) && (mmio_base_phys != 0))
			break;
	}
	if (mmio)
	{
		mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
		//		RA->mmio_base = rinfo->mmio_base;
	} 
	else
	{
		IOLog("FakeSMC_Radeon: have no mmio\n ");
		return false;
	}
	
	
	m_Binding = new Binding(this);
	char value[2];
	value[0] = 10;
	value[1] = 0;
	//	FakeSMCAddKeyCallback("TG0P", "sp78", 2, value, &Update);
	FakeSMCAddKey("TG0D", "sp78", 2, value, m_Binding);
	
	InfoLog(" key TG0D registered\n");	
	RadeonService = this;
/*	
	registerService(0);
	TZWorkLoop = getWorkLoop();
	TZPollTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &RadeonPlugin::poller));
	if (!TZWorkLoop || !TZPollTimer || (kIOReturnSuccess != TZWorkLoop->addEventSource(TZPollTimer))) return false;
	poller();
	IOLog("Fx3100: timer started\n");	
*/
	return true;	
	
}
/*
IOReturn RadeonPlugin::poller( void )
{
	TZPollTimer->setTimeoutMS(kTimeoutMSecs);	
	return(ret);
}
*/
bool RadeonPlugin::init(OSDictionary *properties)
{    
	return super::init(properties);
}

void RadeonPlugin::stop (IOService* provider)
{
	//	FakeSMCRemoveKeyCallback("TG0P");
	FakeSMCRemoveKeyBinding("TG0D");
	delete m_Binding;
	
	super::stop(provider);
}

void RadeonPlugin::free ()
{
	super::free ();
}
