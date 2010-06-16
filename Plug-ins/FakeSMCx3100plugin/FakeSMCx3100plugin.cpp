/* 
 *  FakeSMC x3100 GPU temperature plugin
 *  Created by Slice 28.05.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCx3100plugin.h"

#define kTimeoutMSecs 1000
#define fVendor "vendor-id"
#define fDevice "device-id"
//#define fArch  "NVDA,current_arch"
#define kIOPCIConfigBaseAddress0 0x10
#define kMCHBAR	0x48
#define TSC1	0x1001
#define TSS1	0x1004
#define TR1		0x1006
#define RTR1	0x1008
#define TIC1	0x100B
#define TSC2	0x1041
#define TSS2	0x1044
#define TR2		0x1046
#define RTR2	0x1048
#define TIC2	0x104B


#define INVID8(offset) (mmio_base[offset])
#define INVID16(offset) OSReadLittleInt16((mmio_base), offset)
#define INVID(offset) OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((mmio_base), offset, val)

OSDefineMetaClassAndStructors(x3100plugin, IOService) //IOPCIDevice)

x3100plugin * x3100service;
//static int count=0;

#define super IOService
void x3100plugin::Update(const char* key, char* data) {	
	if(CompareKeys(key, "TG0D"))
	{
		short value;
			if (mmio_base) {
				OUTVID(TIC1, 3);
				//		if ((INVID16(TSC1) & (1<<15)) && !(INVID16(TSC1) & (1<<8)))//enabled and ready
				for (int i=0; i<1000; i++) {  //attempts to ready
					
					if (INVID16(TSS1) & (1<<10))   //valid?
						break;
					IOSleep(10);
				}				
				value = INVID8(TR1);
			}				
				
		data[0] = 140 - value;
		data[1] = 0;
	} 
}
#if 0
int x3100plugin::update(int keyN)
{	
	short value;  //unused
	
	if (mmio_base) {
/*		
		if (count < 8) {
			IOLog("Fx3100: update check keyN=%d\n", keyN);
			for (int i=0; i<0x2f; i +=16) {
				IOLog("%04lx: ", (long unsigned int)i+0x1000);
				for (int j=0; j<16; j += 1) {
					IOLog("%02lx ", (long unsigned int)INVID8(i+j+0x1000));
				}
				IOLog("\n");
			}
			count++;
		}
*/		
		if(!keyN)
		{
			OUTVID(TIC1, 3);
			//		if ((INVID16(TSC1) & (1<<15)) && !(INVID16(TSC1) & (1<<8)))//enabled and ready
			for (int i=0; i<1000; i++) {  //attempts to ready
		   
				if (INVID16(TSS1) & (1<<10))   //valid?
					break;
				IOSleep(10);
			}				
			value = INVID8(TR1);
/*			
			if (count < 8) {
				IOLog("Fx3100: value check keyN=%d TR=%d\n", keyN, value);
			}			
*/			 
		}
/*		else 
		{
			OUTVID(TIC2, 3);
			if ((INVID16(TSC2) & (1<<15)) && !(INVID16(TSC2) & (1<<8))) {   //enabled and ready
				for (int i=0; i<1000; i++) {  //attempts to ready
					if (INVID16(TSS1) & (1<<10))   //valid?
						break;
					IODelay(10000);
				}				
				value = INVID8(TR2);		
				if (count < 8) {
					IOLog("Fx3100: value check keyN=%d TR=%d\n", keyN, value);
			}
		}
	}
 */
	}
	return(127 - value);
}
#endif

IOService*
x3100plugin::probe(IOService *provider, SInt32 *score)
{
#if DEBUG	
	OSData*		prop;
	UInt32		Vchip;
	
	prop = OSDynamicCast( OSData , provider->getProperty(fVendor)); // safe way to get vendor
	if(prop)
	{
		Vchip = *(UInt32*) prop->getBytesNoCopy();
		IOLog("Fx3100: found %lx chip\n", (long unsigned int)Vchip);
		if( (Vchip & 0xffff) != 0x8086) //check if vendorID is really Intel, if not don't bother
		{
			//IOLog("Can't Find Intel Chip!\n");
			return( 0 );
		}		
	}
#endif	
    if( !super::probe( provider, score ))
		return( 0 );
	//	IOLog("Fx3100: probe success\n");	
	return (this);
}

bool
x3100plugin::start( IOService * provider ) {
	if(!provider || !super::start(provider))
		return false;
	//	OSData*		idKey;	
	//	IOLog("Fx3100: starting\n");	

	
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
		IOMemoryDescriptor *		theDescriptor;
		IOPhysicalAddress bar = (IOPhysicalAddress)((VCard->configRead32(kMCHBAR)) & ~0xf);
//		IOLog("Fx3100: register space=%08lx\n", (long unsigned int)bar);
		theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x2000, kIODirectionOutIn); // | kIOMapInhibitCache);
		if(theDescriptor != NULL)
		{
			mmio = theDescriptor->map ();
			if(mmio != NULL)
			{
				//		UInt32 addr = map->getPhysicalAddress();
				mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
#if 0				
				InfoLog(" MCHBAR mapped\n");
				for (int i=0; i<0x2f; i +=16) {
					IOLog("%04lx: ", (long unsigned int)i+0x1000);
					for (int j=0; j<16; j += 1) {
						IOLog("%02lx ", (long unsigned int)INVID8(i+j+0x1000));
					}
					IOLog("\n");
				}
				//mmio->release();
#endif				
			}
			else
			{
				InfoLog(" MCHBAR failed to map\n");
				return -1;
			}			
		}	
	
	m_Binding = new Binding(this);
	char value[2];
	value[0] = 10;
	value[1] = 0;
	//	FakeSMCAddKeyCallback("TG0P", "sp78", 2, value, &Update);
	FakeSMCAddKey("TG0D", "sp78", 2, value, m_Binding);
	
	InfoLog(" key TG0D registered\n");	
	x3100service = this;
/*	
	registerService(0);
	TZWorkLoop = getWorkLoop();
	TZPollTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &x3100plugin::poller));
	if (!TZWorkLoop || !TZPollTimer || (kIOReturnSuccess != TZWorkLoop->addEventSource(TZPollTimer))) return false;
	poller();
	IOLog("Fx3100: timer started\n");	
*/
	return true;	
	
}
/*
IOReturn x3100plugin::poller( void )
{
	TZPollTimer->setTimeoutMS(kTimeoutMSecs);	
	return(ret);
}
*/
bool x3100plugin::init(OSDictionary *properties)
{    
	return super::init(properties);
}

void x3100plugin::stop (IOService* provider)
{
	//	FakeSMCRemoveKeyCallback("TG0P");
	FakeSMCRemoveKeyBinding("TG0D");
	delete m_Binding;
	
	super::stop(provider);
}

void x3100plugin::free ()
{
	super::free ();
}
