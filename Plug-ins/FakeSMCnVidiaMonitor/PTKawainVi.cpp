/*
 *	FakeSMC nVidia plugin
 *	Created by гык-sse2
 *	No rights reserved
 */
#include "PTKawainVi.h"
#include "fakesmc.h"

#define INVID(offset) OSReadLittleInt32((nvio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((nvio_base), offset, val)
OSDefineMetaClassAndStructors(PTKawainVi, IOService)



PTKawainVi* PTKawainViService;

int PTKawainVi::CalcSpeed_nv50(int base_freq, int m1, int m2, int n1, int n2, int p)
{
	return (int)((float)(n1*n2)/(m1*m2) * base_freq) >> p;
}

float PTKawainVi::GetClock_nv50(int base_freq, unsigned int pll, unsigned int pll2)
{
	int m1, m2, n1, n2, p;
	
	p = (pll >> 16) & 0x03;
	m1 = pll2 & 0xFF;
	n1 = (pll2 >> 8) & 0xFF;
	
	// This is always 1 for NV5x? 
	m2 = 1;
	n2 = 1;
	
	IOLog("m1=%d m2=%d n1=%d n2=%d p=%d\n", m1, m2, n1, n2, p);
	
	// The clocks need to be multiplied by 4 for some reason. Is this 4 stored in 0x4000/0x4004? 
	return (float)4*CalcSpeed_nv50(base_freq, m1, m2, n1, n2, p)/1000;
}

float PTKawainVi::nv50_get_gpu_speed()
{
	int pll = INVID(0x4028);
	int pll2 = INVID(0x402c);
	IOLog("NVPLL_COEFF=%08x\n", pll);
	IOLog("NVPLL2_COEFF=%08x\n", pll2);
		
	//return (float)GetClock_nv50(nv_card->base_freq, pll, pll2);
	return 0;
}


float PTKawainVi::g84_get_fanspeed()
{
	int pwm_divider = INVID(0xe11c) & 0x7fff;
	
	//* On most Geforce8/9 cards I have seen the fanspeed register is 'inverted', so
	 //  a low value corresponds with fullspeed (to be exact the register defines the low
	 //  period of a pwm pulse. Though some boards aren't inverted like a 8500GT (G86). I'm
	 //  not sure what we should do about this. If it is possible to whitelist some generations
	 //  or so we should perhaps do that or perhaps there is some setting in the bios? So right
	//  now 100% would show 0% on a 8500GT.
	//
	//  Further some boards use 0xe114 / 0xe118 instead of 0xe11c / 0xe1220. At least the 9800GTX
	//  seems to do that. When I have a more clear picture of the situation those should receive support too.
	 //
	float fanspeed = (float)(pwm_divider - (INVID(0xe120) & 0x7fff)) * 100.0/(float)pwm_divider;
	return fanspeed;
}


// Fanspeed code for Geforce6800 hardware 
float PTKawainVi::nv40_get_fanspeed()
{
	// Bit 30-16 of register 0x10f0 control the voltage for the pwm signal generator
	//  which is connected to the fan. By changing the value in the register the duty cycle can be controlled
	//  so that the fan turns slower or faster. Bit 14-0 of 0x10f0 contain the pwm division
	//  ratio which decides the smallest fanspeed adjustment step.
	//  The value stored in the registers needs to be inverted, so a value of 10% means 90% and so on.
	//
	int pwm_divider = INVID(0x10f0) & 0x7fff;
	float fanspeed = (float)(pwm_divider - ((INVID(0x10f0) >> 16) & 0x7fff)) * 100.0/(float)pwm_divider;
	return fanspeed;
}

// Fanspeed code for Geforce6600 hardware (does this work for 6200 cards too??)
float PTKawainVi::nv43_get_fanspeed()
{
	// The first 12 or more bits of register 0x15f4 control the voltage for the pwm signal generator in case
	//  of Geforce 6200/6600(GT)/7600/7800GS hardware. By changing the value in the register the duty cycle of the pwm signal
	//  can be controlled so that the fan turns slower or faster. The first part of register 0x15f8 contains the pwm division ratio.
	//  The value stored in the registers needs to be inverted, so a value of 10% means 90% and so on. (pwm_divider means off, 0 means on)
	//
	int pwm_divider = INVID(0x15f8) & 0x3fff;
	float fanspeed = (pwm_divider - (INVID(0x15f4) & 0x3fff)) * 100.0/(float)pwm_divider;
	return fanspeed;
}

int PTKawainVi::CalcSpeed_nv40(int base_freq, int m1, int m2, int n1, int n2, int p)
{
	return (int)((float)(n1*n2)/(m1*m2) * base_freq) >> p;
}

float PTKawainVi::GetClock_nv40(int base_freq, unsigned int pll, unsigned int pll2)
{
	int m1, m2, n1, n2, p;
	
	// mpll at 0x4020 and 0x4024; nvpll at 0x4000 and 0x4004 
	p = (pll >> 16) & 0x03;
	m1 = pll2 & 0xFF;
	n1 = (pll2 >> 8) & 0xFF;
	
	// Bit 8 of the first pll register can be used to disable the second set of dividers/multipliers. 
	if(pll & 0x100)
	{
		m2 = 1;
		n2 = 1;
	}
	// NV46/NV49/NV4B cards seem to use a different calculation; I'm not sure how it works yet, so for now check the architecture. Further it looks like bit 15 can be used to identify it but I'm not sure yet.
	else if(pll & 0x1000)
	{
		m2 = 1;
		n2 = 1;
	}
	else
	{
		m2 = (pll2 >> 16) & 0xFF;
		n2 = (pll2 >> 24) & 0xFF;
	}
	
	IOLog("m1=%d m2=%d n1=%d n2=%d p=%d\n", m1, m2, n1, n2, p);
	
	return (float)CalcSpeed_nv40(base_freq, m1, m2, n1, n2, p)/1000;
}

float PTKawainVi::nv40_get_gpu_speed()
{
	int pll = INVID(0x4000);
	int pll2 = INVID(0x4004);
	IOLog("NVPLL_COEFF=%08x\n", pll);
	IOLog("NVPLL2_COEFF=%08x\n", pll2);	
	//return (float)GetClock_nv40(nv_card->base_freq, pll, pll2);
	return 0;
}


static void UpdateT(const char* key, char* data) {
	PTKawainViService->UpdateT(key, data);
}

static void UpdateF(const char* key, char* data) {
	PTKawainViService->UpdateF(key,data);
}

void PTKawainVi::UpdateT(const char* key, char* data) {	
	if (nvio) {
		switch (arch) {
			case 0x43:
			case 0x44:
			case 0x47:
			case 0x46: /* are these really the default ones? they come from a 7300GS bios */
			case 0x49: /* are these really the default ones? they come from a 7900GT/GTX bioses */
			case 0x4b: /* are these really the default ones? they come from a 7600GT bios */
				data[0] = (INVID(0x15b4) & 0x1fff)*slope+offset;
				break;
			case 0x50:
				data[0] = (INVID(0x20008) & 0x1fff)*slope+offset;
				break;
			case 0x84:
			case 0x86:
			case 0x94:
			case 0x96:
			case 0x200:
				data[0] = (INVID(0x20400));
				break;
			case 0x92:
				data[0] = ((INVID(0x20008) & 0x1fff)+offset)/slope;
				break;	
			default:
				data[0]=0;
		}
		data[1]=0;
	}
}

void PTKawainVi::UpdateF(const char* key, char* data) {
	if (nvio) {
		int nVfan;
		switch (arch) {
			case 0x40:
			case 0x49:
				nVfan=int(nv40_get_fanspeed())<<2;
				break;
			case 0x41:
			case 0x43:
			case 0x44:
			case 0x47:
			case 0x4b: 
				nVfan=int(nv43_get_fanspeed())<<2;
			case 0x50:
			case 0x84:
			case 0x86:
			case 0x94:
			case 0x96:
			case 0x92:
			//case 0x200:
				nVfan=int(g84_get_fanspeed())<<2;
			default:
				nVfan=0;
		}
		data[0]=nVfan>>8;
		data[1]=nVfan&0xff;
	}
}

IOService*
PTKawainVi::probe(IOService *provider, SInt32 *score) {
	PTKawainViService=this;
	return IOService::probe(provider,score);
}

bool
PTKawainVi::start( IOService * provider ) {
	bool res=IOService::start(provider);
	NVCard=NULL;
	IOLog("PTKawainVi: started\n");
	OSData* idKey;
	OSDictionary * iopci = serviceMatching("IOPCIDevice");
	OSString* str;
	if (iopci) {
		OSIterator * iterator = getMatchingServices(iopci);
		if (iterator) {
			while (NVCard = OSDynamicCast(IOPCIDevice, iterator->getNextObject())) {
				vendor_id=0;
				str=OSDynamicCast(OSString, NVCard->getProperty("IOName"));
				idKey=OSDynamicCast(OSData, NVCard->getProperty("vendor-id"));
				if (idKey)
					vendor_id=*(UInt32*)idKey->getBytesNoCopy();
				if ((str->isEqualTo("display"))&&(vendor_id==0x10de)){
					break;
				}
			}
		}
	}
	if (!NVCard) {
		IOLog("PTKawainVi: nVidia graphics adapter not found\n");
		return false;
	}
	
	NVCard->setMemoryEnable(true);
	nvio = NVCard->mapDeviceMemoryWithIndex(0);		
	nvio_base = (volatile UInt8 *)nvio->getVirtualAddress();
	idKey=OSDynamicCast(OSData, NVCard->getProperty("device-id"));
	if (idKey)
		device_id=*(UInt32*)idKey->getBytesNoCopy();
	IOLog("NVDA Device ID:%x\n",device_id);
	this->get_gpu_arch();
	IOLog("NVDA Architecture: NV%x\n", arch);
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
		case 0x46:  //are these really the default ones? they come from a 7300GS bios 
			offset = -24775.0/100.0;
			slope = 467.0/10000.0;
			break;
		case 0x49:  //are these really the default ones? they come from a 7900GT/GTX bioses 
			offset = -25051.0/100.0;
			slope = 458.0/10000.0;
			break;
		case 0x4b:  //are these really the default ones? they come from a 7600GT bios 
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
		case 0x200:
			//Break, but not return false
			break;
		case 0x92:
			offset = -13115 + 18.7;
			slope = 18.7;
			break;
		default:
			return false;
	}
	
	
	char value[2];
	
	FakeSMCAddKeyCallback("TG0D", "sp78", 2, value, &::UpdateT);
	FakeSMCAddKeyCallback("F5Ac", "fp2e", 2, value, &::UpdateF);
	UpdateFNum(1);
	return res;
		
}

bool
PTKawainVi::init(OSDictionary *properties) {
    return IOService::init(properties);
}

void PTKawainVi::stop (IOService* provider) {
	IOService::stop (provider);
	FakeSMCRemoveKeyCallback("TG0D");
	FakeSMCRemoveKeyCallback("F5Ac");
}

void PTKawainVi::free () {
	IOService::free ();
}

void PTKawainVi::get_gpu_arch() {
	switch(device_id & 0xff0) {
		case 0x20:
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
			break;
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
		case 0x5e0: // GT2x0 
		case 0x5f0:  //GT2x0 
			arch = 0x200;
			break;
		case 0x6e0:// G98 
		case 0x6f0: // G98 
		case 0x860: // C79 
			arch = 0x86;
			break;
		case 0x600: // G92 
		case 0x610: // G92 
			arch = 0x92;
			break;
		case 0x620: // 9600GT 'G94'/
			arch = 0x94;
			break;
		case 0x640: // 9500GT 
			arch = 0x96;
			break;
		case 0x240:
		case 0x3d0: // not sure if this is a C51 too
		case 0x530: //* not sure if the 70xx is C51 too 
			arch = 0x51;
			break;
		case 0x2e0:
		case 0xf0:
			//* The code above doesn't work for pci-express cards as multiple architectures share one id-range */
			switch(device_id)
		{
			case 0xf0: //* 6800
			case 0xf9: //* 6800Ultra 
						arch = 0x40;
				break;
			case 0xf6: //* 6800GS/XT *
				arch = 0x41;
				break;
			case 0xf1: //* 6600/6600GT *
			case 0xf2: //* 6600GT *
			case 0xf3: //* 6200 *
			case 0xf4: //* 6600LE *
				arch = 0x43;
				break;
			case 0xf5: //* 7800GS *
				arch = 0x47;
				break;
			case 0xfa: //* PCX5700 *
				arch = 0x31;
				break;
			case 0xf8: //* QuadroFX 3400 *
			case 0xfb: //* PCX5900 *
				arch = 0x35;
				break;
			case 0xfc: //* PCX5300 */
			case 0xfd: //* Quadro NVS280/FX330, FX5200 based? *
			case 0xff: //* PCX4300 *
				arch = 0x25;
				break;
			case 0xfe: //* Quadro 1300, has the same id as a FX3000 *
				arch = 0x35;
				break;
			case 0x2e0: //* Geforce 7600GT AGP (at least Leadtek uses this id) *
			case 0x2e1: //* Geforce 7600GS AGP (at least BFG uses this id) *
			case 0x2e2: //* Geforce 7300GT AGP (at least a Galaxy 7300GT uses this id) *
				arch = 0x4B;
				break;
				case 0x2e4: //* Geforce 7950 GT AGP *
					arch = 0x49;
					break;
			}
			break;
		default:
			arch = 0;
	}
}
