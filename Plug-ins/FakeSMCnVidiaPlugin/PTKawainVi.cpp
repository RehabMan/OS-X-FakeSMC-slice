/*
 *	FakeSMC nVidia plugin
 *	Created by гык-sse2
 *	No rights reserved
 */
#include "PTKawainVi.h"

#define INVID(offset) OSReadLittleInt32((nvio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((nvio_base), offset, val)
OSDefineMetaClassAndStructors(PTKawainVi, IOPCIDevice)

static int nVtemp;
static void Update(SMCData node) {	
	node->data[0] = nVtemp;
	node->data[1] = 0;
}



IOService*
PTKawainVi::probe(IOService *provider, SInt32 *score) {
	NVCard = (IOPCIDevice*)provider;
	NVCard->setMemoryEnable(true);
	nvio = NVCard->mapDeviceMemoryWithIndex(0);		
	nvio_base_phys = nvio->getPhysicalAddress();
	OSData* idKey;
	char* p;
	idKey=OSDynamicCast(OSData, NVCard->getProperty("device-id"));
	if (idKey)
		p=(char*)idKey->getBytesNoCopy();
	device_id=p[1]&0xff;
	device_id<<=8;
	device_id|=(p[0]&0xff);
	//IOLog("NVDA Device ID:%x\n",device_id);
	this->get_gpu_arch();
	IOLog("NVDA Archicture: NV%x\n", arch);
	switch(arch) {
		case 0x43:
			offset = 32060.0/1000.0;
			slope = 792.0/1000.0;
			break;
		case 0x44:
		case 0x47:
			offset = 27839.0/1000.0;
			slope = 700.0/1000.0;
			break;
		case 0x46: /* are these really the default ones? they come from a 7300GS bios */
			offset = -24775.0/100.0;
			slope = 467.0/10000.0;
			break;
		case 0x49: /* are these really the default ones? they come from a 7900GT/GTX bioses */
			offset = -25051.0/100.0;
			slope = 458.0/10000.0;
			break;
		case 0x4b: /* are these really the default ones? they come from a 7600GT bios */
			offset = -24088.0/100.0;
			slope = 442.0/10000.0;
			break;
		case 0x50:
			offset = -227.0;
			slope = 430.0/10000.0;
			break;
		case 0x84:
		case 0x86:
		case 0x94:
		case 0x96:
			//Break, but not return false
			break;
		case 0x92:
			offset = -13115 + 18.7;
			slope = 18.7;
			break;
		default:
			return false;
		}
	return IOPCIDevice::probe(provider,score);
}

bool
PTKawainVi::start( IOService * provider ) {
	char value[2];
	FakeSMCRegisterKey("TG0D", 2, value, &Update);
	WorkLoop = IOWorkLoop::workLoop();
	TimerEventSource = IOTimerEventSource::timerEventSource(this,OSMemberFunctionCast(IOTimerEventSource::Action, this, &PTKawainVi::LoopTimerEvent));
	if (!TimerEventSource || !WorkLoop || (kIOReturnSuccess != WorkLoop->addEventSource(TimerEventSource))) 
		return false;
	LoopTimerEvent();
	IOLog("PTKawainVi: started\n");
	return IOPCIDevice::start(provider);
		
}

bool
PTKawainVi::init(OSDictionary *properties) {
    IOPCIDevice::init(properties);
	return true;
}

void PTKawainVi::stop (IOService* provider) {
	IOPCIDevice::stop (provider);
	FakeSMCUnregisterKey("TG0D");
	TimerEventSource->disable();
	TimerEventSource->release();
}

IOReturn PTKawainVi::LoopTimerEvent (void) {
	if (nvio) {
		nvio_base = (volatile UInt8 *)nvio->getVirtualAddress();		
		switch (arch) {
			case 0x43:
			case 0x44:
			case 0x47:
			case 0x46: /* are these really the default ones? they come from a 7300GS bios */
			case 0x49: /* are these really the default ones? they come from a 7900GT/GTX bioses */
			case 0x4b: /* are these really the default ones? they come from a 7600GT bios */
				nVtemp = (INVID(0x15b4) & 0x1fff)*slope+offset;
				break;
			case 0x50:
				nVtemp = (INVID(0x200008) & 0x1fff)*slope+offset;
				break;
			case 0x84:
			case 0x86:
			case 0x94:
			case 0x96:
				nVtemp = (INVID(0x20400));
				break;
			case 0x92:
				nVtemp = ((INVID(0x20008) & 0x1fff)+offset)/slope;
				break;	
		}
	}
	TimerEventSource->setTimeoutMS(1000);
	return kIOReturnSuccess;
}

void PTKawainVi::free () {
	IOPCIDevice::free ();
}

void PTKawainVi::get_gpu_arch() {
	switch(device_id & 0xff0) {
		/*case 0x20:
			arch = 0x5;
			break;
		case 0x100:
		case 0x110:
		case 0x150:
		case 0x1a0:
			arch = 0x10;
			break;
		case 0x170:
		case 0x180:
		case 0x1f0:
			arch = 0x17;
			break;
		case 0x200:
			arch = 0x20;
			break;
		case 0x250:
		case 0x280:
		case 0x320:	// We don't treat the FX5200/FX5500 as FX cards/
			arch = 0x25;
			break;
		case 0x300:
			arch = 0x30;
			break;
		case 0x330:
			arch = 0x35; // Similar to NV30 but fanspeed stuff works differently
			break;
			// Give a seperate arch to FX5600/FX5700 cards as they need different code than other FX cards
		case 0x310:
		case 0x340:
			arch = 0x31;
			break;*/
		case 0x40:
		case 0x120:
		case 0x130:
		case 0x210:
		case 0x230:
			arch = 0x40;
			break;
		case 0xc0:
			arch = 0x41;
			break;
		case 0x140:
			arch = 0x43; // Similar to NV40 but with different fanspeed code
			break;
		case 0x160:
		case 0x220:
			arch = 0x44;
			break;
		case 0x1d0:
			arch = 0x46; //7300LE/GS
			break;
		case 0x90:
			arch = 0x47;
			break;
		case 0x290:
			arch = 0x49; // 7900
			break;
		case 0x390:
			arch = 0x4b; // 7600
			break;
		case 0x190:
			arch = 0x50; // 8800 'NV50 / G80'
			break;
		case 0x400: // 8600 'G84'
			arch = 0x84;
			break;
		case 0x420: // 8500 'G86'
			arch = 0x86;
			break;
		case 0x5e0: /* GT2x0 */
		case 0x5f0: /* GT2x0 */
			arch = 0x200;
			break;
		case 0x6e0: /* G98 */
		case 0x6f0: /* G98 */
		case 0x860: /* C79 */
			arch = 0x86;
			break;
		case 0x600: /* G92 */
		case 0x610: /* G92 */
			arch = 0x92;
			break;
		case 0x620: /* 9600GT 'G94' */
			arch = 0x94;
			break;
		case 0x640: /* 9500GT */
			arch = 0x96;
			break;
		case 0x240:
		case 0x3d0: /* not sure if this is a C51 too */
		case 0x530: /* not sure if the 70xx is C51 too */
			arch = 0x51;
			break;
		case 0x2e0:
		case 0xf0:
			/* The code above doesn't work for pci-express cards as multiple architectures share one id-range */
			switch(device_id)
		{
			case 0xf0: /* 6800 */
			case 0xf9: /* 6800Ultra */
				arch = 0x40;
				break;
			case 0xf6: /* 6800GS/XT */
				arch = 0x41;
				break;
			case 0xf1: /* 6600/6600GT */
			case 0xf2: /* 6600GT */
			case 0xf3: /* 6200 */
			case 0xf4: /* 6600LE */
				arch = 0x43;
				break;
			case 0xf5: /* 7800GS */
				arch = 0x47;
				break;
			case 0xfa: /* PCX5700 */
				arch = 0x31;
				break;
			case 0xf8: /* QuadroFX 3400 */
			case 0xfb: /* PCX5900 */
				arch = 0x35;
				break;
			case 0xfc: /* PCX5300 */
			case 0xfd: /* Quadro NVS280/FX330, FX5200 based? */
			case 0xff: /* PCX4300 */
				arch = 0x25;
				break;
			case 0xfe: /* Quadro 1300, has the same id as a FX3000 */
				arch = 0x35;
				break;
			case 0x2e0: /* Geforce 7600GT AGP (at least Leadtek uses this id) */
			case 0x2e1: /* Geforce 7600GS AGP (at least BFG uses this id) */
			case 0x2e2: /* Geforce 7300GT AGP (at least a Galaxy 7300GT uses this id) */
				arch = 0x4B;
				break;
				case 0x2e4: /* Geforce 7950 GT AGP */
					arch = 0x49;
					break;
			}
			break;
		default:
			arch = 0;
	}
}

