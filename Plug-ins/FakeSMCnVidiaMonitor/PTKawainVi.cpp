/*
 *	FakeSMC nVidia plugin
 *	Created by гык-sse2
 *	No rights reserved
 */
#include "PTKawainVi.h"
#include "fakesmc.h"

uint16_t
_OSReadInt8(
			 const volatile void               * base,
			 uintptr_t                     byteOffset
			 )
{
    return *(volatile uint8_t *)((uintptr_t)base + byteOffset);
}


#define INVID(offset) OSReadLittleInt32((nvio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((nvio_base), offset, val)
#define PROMIN(offset) _OSReadInt8((nvio_base+0x300000), offset)
#define READ_BYTE(bios, offset)  (bios[offset]&0xff)
#define READ_SHORT(bios, offset) ((bios[offset+1]&0xff) << 8 | (bios[offset]&0xff))
#define READ_INT(bios, offset) ((bios[offset+3]&0xff) << 24 | (bios[offset+2]&0xff) << 16 | (bios[offset+1]&0xff) << 8 | (bios[offset]&0xff))
#define READ_LONG(bios, offset) (READ_INT(bios, offset+4)<<32 | READ_INT(bios, offset))


OSDefineMetaClassAndStructors(PTKawainVi, IOService)




PTKawainVi* PTKawainViService;

bool PTKawainVi::is4x() {
	switch (arch) {
		case 0x40:
		case 0x49:
		case 0x41:
		case 0x43:
		case 0x44:
		case 0x46:
		case 0x47:
		case 0x4b: 
			return true;
	}
	return false;
}

bool PTKawainVi::is5x() {
	switch (arch) {
		case 0x50:
		case 0x84:
		case 0x86:
		case 0x94:
		case 0x96:
			return true;
	}
	return false;
}

unsigned int PTKawainVi::locate(const char *str, int offset) {
	int size = strlen(str);
	int i;
	char* data;
	
	/* We shouldn't assume this is allways 64kB */
	for(i=offset; i<0xffff; i++) {
		data = (char*)&bios[i];
		if(strncmp(data, str, size) == 0)
			return i;
	}
	return 0;
}


bool PTKawainVi::parse_bios() {
	unsigned short bit_offset = 0;
	//unsigned short nv_offset = 0;
	unsigned short pcir_offset = 0;
	//unsigned short device_id = 0;
	//struct nvbios *bios;
	//int i=0;
	
	/* All bioses start with this '0x55 0xAA' signature */
	if((bios[0] != 0x55) || (bios[1] != (char)0xAA))
		return false;
	
	/* Fail when the PCIR header can't be found; it is present on all PCI bioses */
	if(!(pcir_offset = locate("PCIR", 0)))
		return false;
	
	/* Fail if the bios is not fbios an Nvidia card */
	if(READ_SHORT(bios, pcir_offset + 4) != 0x10de)
		return false;
	
	if(is4x()||is5x()) {
		/* For NV40 card the BIT structure is used instead of the BMP structure (last one doesn't exist anymore on 6600/6800le cards). */
		if(!(bit_offset = locate("BIT", 0)))
			return false;
		
		parse_bit_structure(bit_offset);
	}
	return true;
}


void PTKawainVi::parse_bit_structure(unsigned int bit_offset) {
	unsigned short init_offset=0;
	unsigned short perf_offset=0;
	unsigned short pll_offset=0;
	unsigned short signon_offset=0;
	unsigned short temp_offset=0;
	unsigned short volt_offset=0;
	unsigned short offset=0;
	
	struct bit_entry {
		unsigned char id[2]; // first byte is ID, second byte sub-ID? 
		unsigned short len; // size of data pointed to by offset 
		unsigned short offset; // offset of data 
	} *entry;
	
	// In older nvidia bioses there was some start position and at fixed positions fbios there offsets to various tables were stored.
	 //  For Geforce6 bioses this is all different. There is still some start position (now called BIT) but offsets to tables aren't at fixed
	 //  positions fbios the start. There's now some weird pattern which starts a few places fbios the start of the BIT section.
	 //  This pattern seems to consist of a subset of the alphabet (all in uppercase). After each such token there is the length of the data
	 // referred to by the entry and an offset. The first entry "0x00 0x01" is probably somewhat different since the length/offset info
	 // seems to be a bit strange. The list ends with the entry "0x00 0x00"
	 
	
	// skip 'B' 'I' 'T' '\0' 
	offset = bit_offset + 4;
	
	// read the entries 
	while (1) {
		entry = (struct bit_entry *)&bios[offset];
		if ((entry->id[0] == 0) && (entry->id[1] == 0))
			break;
		
		switch (entry->id[0]) {
			case 'B': // BIOS related data 
				//bios->version = nv40_bios_version_to_str(bios, entry->offset);
				break;
			case 'C': // Configuration table; it contains at least PLL parameters 
				pll_offset = READ_SHORT(bios, entry->offset + 8);
				//parse_bit_pll_table(bios, bios, pll_offset);
				break;
			case 'I': // Init table 
				init_offset = READ_SHORT(bios, entry->offset);
				//parse_bit_init_script_table(bios, bios, init_offset, entry->len);
				break;
			case 'P': // Performance related data 
				perf_offset = READ_SHORT(bios, entry->offset);
				//parse_bit_performance_table(bios, bios, perf_offset);
				
				temp_offset = READ_SHORT(bios, entry->offset + 0xc);
				parse_bit_temperature_table(temp_offset);
				
				// 0x10 behind perf_offset the voltage table offset is stored
				volt_offset = READ_SHORT(bios, entry->offset + 0x10);
				//parse_voltage_table(bios, bios, volt_offset);
				break;
			case 'S':
				// table with string references of signon-message,
				 //BIOS version, BIOS copyright, OEM string, VESA vendor,
				 //VESA Product Name, and VESA Product Rev.
				 //table consists of offset, max-string-length pairs
				 //for all strings 
				signon_offset = READ_SHORT(bios, entry->offset);
				//bios->signon_msg = nv_read(bios, signon_offset);
				break;
		}
		offset += sizeof(struct bit_entry);
	}
}


void PTKawainVi::parse_bit_temperature_table(int offset) {
	short i;
	BitTableHeader *header = (BitTableHeader*)&bios[offset];
	float diode_offset_mult=0, diode_offset_div=1, slope_mult=0, slope_div=1;
	/*switch(arch) {
		case 0x43:
			diode_offset_mult = 32060;
			diode_offset_div = 1000;
			slope_mult = 792;
			slope_div = 1000;
			break;
		case 0x44:
		case 0x47:
			diode_offset_mult = 27839;
			diode_offset_div = 1000;
			slope_mult = 780;
			slope_div = 1000;
			break;
		case 0x46: // are these really the default ones? they come fbios a 7300GS bios 
			diode_offset_mult = -24775;
			diode_offset_div = 100;
			slope_mult = 467;
			slope_div = 10000;
			break;
		case 0x49: // are these really the default ones? they come fbios a 7900GT/GTX bioses 
			diode_offset_mult = -25051;
			diode_offset_div = 100;
			slope_mult = 458;
			slope_div = 10000;
			break;
		case 0x4b: // are these really the default ones? they come fbios a 7600GT bios
			diode_offset_mult = -24088;
			diode_offset_div = 100;
			slope_mult = 442;
			slope_div = 10000;
			break;
	}*/
	
	offset += header->start;
	for(i=0; i<header->num_entries; i++)
	{
		unsigned char id = bios[offset];
		short value = READ_SHORT(bios, offset+1);;
		
		switch(id)
		{
				// The temperature section can store settings for more than just the builtin sensor.
				//  The value of 0x0 sets the channel for which the values below are meant. Right now
				//  we ignore this as we only use option 0x10-0x13 which are specific to the builtin sensor.
				//  Further what do 0x33/0x34 contain? Those appear on Geforce7300/7600/7900 cards.
				
			case 0x1:

				IOLog("0x1: (%0x) %d 0x%0x\n", value, (value>>9) & 0x7f, value & 0x3ff);

				if((value & 0x8f) == 0)
					correction = (value>>9) & 0x7f;
				break;
				/* An id of 4 seems to correspond to a temperature threshold but 5, 6 and 8 have similar values, what are they? */
			case 0x4:
			case 0x5:
			case 0x6:
			case 0x8:
				/* IOLog("0x%x: 0x%x %d\n", id, value & 0xf, (value>>4) & 0x1ff); */
				break;
			case 0x10:
				diode_offset_mult = value;
				break;
			case 0x11:
				diode_offset_div = value;
				break;
			case 0x12:
				slope_mult = value;
				break;
			case 0x13:
				slope_div = value;
				break;
			default:
				IOLog("0x%x: %x\n", id, value);
		}
		offset += header->entry_size;
	}
	offset=diode_offset_mult /diode_offset_div;
	slope=slope_mult / slope_div;
	IOLog("temperature table version: %#x\n", header->version);
	IOLog("correction: %d\n", correction);
	IOLog("offset: %d\n", offset);
	IOLog("slope: %d\n", (int)slope);
}

/* Load the video bios from the ROM. Note laptops might not have a ROM which can be accessed from the GPU */
bool PTKawainVi::load_bios_prom() {
	int i;
	
	/* enable bios parsing; on some boards the display might turn off */
	OUTVID(0x1850, 0x0);
	
	for(i=0; i<0xffff; i++) {
		/* On some 6600GT/6800LE boards bios there are issues with the rom.
		 /  Normaly when you want to read data from lets say address X, you receive
		 /  the data when it is ready. For some roms the outputs aren't "stable" yet when
		 /  we want to read out the data. A workaround from Unwinder is to try to access the location
		 /  several times in the hope that the outputs will become stable. In the case of instablity
		 /  each fourth byte was wrong (needs to be shifted 4 to the left) and furhter there was some garbage
		 /
		 /  A delay of 4 extra reads helps for most 6600GT cards but for 6800Go cards atleast 5 are needed.
		 */
		bios[i] = PROMIN(i);
		bios[i] = PROMIN(i);
		bios[i] = PROMIN(i);
		bios[i] = PROMIN(i);
		bios[i] = PROMIN(i);
	}
	
	/* disable the rom; if we don't do it the screens stays black on some cards */
	OUTVID(0x1850, 0x1);
	
	/* Make sure the bios is correct */
	//if(verify_bios(data))
		return true;
	/*else
		return 0;*/
}





int PTKawainVi::CalcSpeed_nv50(int m1, int m2, int n1, int n2, int p) {
	return (int)((float)(n1*n2)/(m1*m2) * base_freq) >> p;
}

float PTKawainVi::GetClock_nv50(unsigned int pll, unsigned int pll2) {
	int m1, m2, n1, n2, p;
	
	p = (pll >> 16) & 0x03;
	m1 = pll2 & 0xFF;
	n1 = (pll2 >> 8) & 0xFF;
	
	// This is always 1 for NV5x? 
	m2 = 1;
	n2 = 1;
	
	//IOLog("m1=%d m2=%d n1=%d n2=%d p=%d\n", m1, m2, n1, n2, p);
	
	// The clocks need to be multiplied by 4 for some reason. Is this 4 stored in 0x4000/0x4004? 
	return (float)4*CalcSpeed_nv50(m1, m2, n1, n2, p)/1000;
}

float PTKawainVi::nv50_get_gpu_speed() {
	int pll = INVID(0x4028);
	int pll2 = INVID(0x402c);
	//IOLog("NVPLL_COEFF=%08x\n", pll);
	//IOLog("NVPLL2_COEFF=%08x\n", pll2);
		
	return (float)GetClock_nv50(pll, pll2);
}


float PTKawainVi::g84_get_fanspeed() {
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
float PTKawainVi::nv40_get_fanspeed() {
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
float PTKawainVi::nv43_get_fanspeed() {
	// The first 12 or more bits of register 0x15f4 control the voltage for the pwm signal generator in case
	//  of Geforce 6200/6600(GT)/7600/7800GS hardware. By changing the value in the register the duty cycle of the pwm signal
	//  can be controlled so that the fan turns slower or faster. The first part of register 0x15f8 contains the pwm division ratio.
	//  The value stored in the registers needs to be inverted, so a value of 10% means 90% and so on. (pwm_divider means off, 0 means on)
	//
	int pwm_divider = INVID(0x15f8) & 0x3fff;
	float fanspeed = (pwm_divider - (INVID(0x15f4) & 0x3fff)) * 100.0/(float)pwm_divider;
	return fanspeed;
}

int PTKawainVi::CalcSpeed_nv40(int m1, int m2, int n1, int n2, int p) {
	return (int)((float)(n1*n2)/(m1*m2) * base_freq) >> p;
}

float PTKawainVi::GetClock_nv40(unsigned int pll, unsigned int pll2) {
	int m1, m2, n1, n2, p;
	
	// mpll at 0x4020 and 0x4024; nvpll at 0x4000 and 0x4004 
	p = (pll >> 16) & 0x03;
	m1 = pll2 & 0xFF;
	n1 = (pll2 >> 8) & 0xFF;
	
	// Bit 8 of the first pll register can be used to disable the second set of dividers/multipliers. 
	if(pll & 0x100) {
		m2 = 1;
		n2 = 1;
	}
	// NV46/NV49/NV4B cards seem to use a different calculation; I'm not sure how it works yet, so for now check the architecture. Further it looks like bit 15 can be used to identify it but I'm not sure yet.
	else if(pll & 0x1000) {
		m2 = 1;
		n2 = 1;
	}
	else {
		m2 = (pll2 >> 16) & 0xFF;
		n2 = (pll2 >> 24) & 0xFF;
	}
	
	//IOLog("m1=%d m2=%d n1=%d n2=%d p=%d\n", m1, m2, n1, n2, p);
	
	return (float)CalcSpeed_nv40(m1, m2, n1, n2, p)/1000;
}

float PTKawainVi::nv40_get_gpu_speed() {
	int pll = INVID(0x4000);
	int pll2 = INVID(0x4004);
	//IOLog("NVPLL_COEFF=%08x\n", pll);
	//IOLog("NVPLL2_COEFF=%08x\n", pll2);	
	return (float)GetClock_nv40(pll, pll2);
}

float PTKawainVi::nv40_get_memory_speed() {
	int pll = INVID(0x4020);
	int pll2 = INVID(0x4024);
	//IOLog("MPLL_COEFF=%08x\n", pll);
	//IOLog("MPLL2_COEFF=%08x\n", pll2);
	return (float)GetClock_nv40(pll, pll2);
}

float PTKawainVi::nv50_get_memory_speed() {
	/* The memory clock appears to be in 0x4008/0x400c, 0x4010/0x4014 and 0x4018/0x401c but the second and third set aren't always set to the same values as 0x4008/0x400c */ 
	int pll = INVID(0x4008);
	int pll2 = INVID(0x400c);
	//IOLog("MPLL_COEFF=%08x\n", pll);
	//IOLog("MPLL2_COEFF=%08x\n", pll2);	
	return (float)GetClock_nv50(pll, pll2);
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
			case 0x46: 
			case 0x49: 
			case 0x4b: 
				data[0] = (INVID(0x15b4) & 0x1fff)*slope+offset+correction;
				break;
			case 0x50:
				data[0] = (INVID(0x20008) & 0x1fff)*slope+offset+correction;
				break;
			case 0x84:
			case 0x86:
			case 0x94:
			case 0x96:
			case 0x92:
				data[0] = (INVID(0x20400));
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
	get_gpu_arch();
	IOLog("NVDA Architecture: NV%x\n", arch);
	load_bios_prom();
	if (!parse_bios()) {
		IOLog("PTKawainVi: Using table values\n");
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
			case 0x46:  //are these really the default ones? they come fbios a 7300GS bios 
				offset = -24775.0/100.0;
				slope = 467.0/10000.0;
				break;
			case 0x49:  //are these really the default ones? they come fbios a 7900GT/GTX bioses 
				offset = -25051.0/100.0;
				slope = 458.0/10000.0;
				break;
			case 0x4b:  //are these really the default ones? they come fbios a 7600GT bios 
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
			case 0x92:
				//Break, but not return false
				break;
			default:
				return false;
		}
	}
	
	char value[2];
	
	FakeSMCAddKeyCallback("TG0D", "sp78", 2, value, &::UpdateT);
	switch (arch) {
		case 0x40:
		case 0x49:
		case 0x41:
		case 0x43:
		case 0x44:
		case 0x47:
		case 0x4b: 
		case 0x50:
		case 0x84:
		case 0x86:
		case 0x94:
		case 0x96:
			char key[5];			
			snprintf(key, 5, "F%dAc", FanIndex = GetFNum());
			FakeSMCAddKeyCallback(key, "fp2e", 2, value, &::UpdateF);
			UpdateFNum(1);
	}
	float gfreq=0, mfreq=0;
	if (is4x()){
		gfreq=nv40_get_gpu_speed();
		mfreq=nv40_get_memory_speed();
	}
	else if (is5x()) {
		gfreq=nv50_get_gpu_speed();
		mfreq=nv50_get_memory_speed();
	}
	IOLog("PTKawainVi: GPU frequency is %d MHz\n",(int)gfreq);
	IOLog("PTKawainVi: VRAM frequency is %d MHz\n", (int)mfreq);
	return res;
		
}

bool
PTKawainVi::init(OSDictionary *properties) {
	base_freq=27000;
    return IOService::init(properties);
}

void PTKawainVi::stop (IOService* provider) {
	IOService::stop (provider);
	FakeSMCRemoveKeyCallback("TG0D");
	switch (arch) {
		case 0x40:
		case 0x49:
		case 0x41:
		case 0x43:
		case 0x44:
		case 0x47:
		case 0x4b: 
		case 0x50:
		case 0x84:
		case 0x86:
		case 0x94:
		case 0x96:
			char key[5];
			snprintf(key, 5, "F%dAc", FanIndex);
			FakeSMCRemoveKeyCallback(key);
			UpdateFNum(-1);
	}
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

