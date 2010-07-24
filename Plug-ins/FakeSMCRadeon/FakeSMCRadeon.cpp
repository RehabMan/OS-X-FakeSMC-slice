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


#define INVID8(offset) (mmio_base[offset])
#define INVID16(offset) OSReadLittleInt16((mmio_base), offset)
#define INVID(offset) OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((mmio_base), offset, val)

#define super IOService

OSDefineMetaClassAndStructors(RadeonPlugin, IOService) 

RadeonPlugin * RadeonService;
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
	//	OSData*		idKey;
	int card_number=0;
	max_card = 1;  //first version for 1 card
	
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
	
	//getRadeonBIOS(); -  anahuya?
	getRadeonInfo();
	switch (rinfo->ChipFamily) {
		case CHIP_FAMILY_R600:
		case CHIP_FAMILY_RV610:
		case CHIP_FAMILY_RV630:
		case CHIP_FAMILY_RV670:
			setup_R6xx(card_number);
			break;
		case CHIP_FAMILY_RV770:
			setup_R7xx(card_number);
			break;
		case CHIP_FAMILY_Evergreen:
			setup_Evergreen(card_number);
			break;
	
		default:
			InfoLog("sorry, but your card %04lx is not supported!\n", chipID>>16);
			break;
	}
	
	RadeonService = this;
	return true;	
	
}
// this is a sample from nVclock project
#if 0
if(nv_card->caps & (GPU_TEMP_MONITORING)){
	snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card_number);
	tempSensor[card_number]=new TemperatureSensor(key, TYPE_SP78, 2);
	
	if(nv_card->caps & (BOARD_TEMP_MONITORING)) {
		snprintf(key, 5, KEY_FORMAT_GPU_BOARD_TEMPERATURE, card_number);
		boardSensor[card_number]=new TemperatureSensor(key, TYPE_SP78, 2);
	}
}



if(nv_card->caps & (I2C_FANSPEED_MONITORING | GPU_FANSPEED_MONITORING)){
	int id=GetNextUnusedKey(KEY_FORMAT_FAN_ID, key);
	int ac=GetNextUnusedKey(KEY_FORMAT_FAN_SPEED, key);
	if (id!=-1 || ac!=-1) {
		int no=id>ac ? id : ac;
		char name[6]; 
		snprintf (name, 6, "GPU %d", card_number);
		snprintf(key, 5, KEY_FORMAT_FAN_ID, no);
		FakeSMCAddKey(key, TYPE_FPE2, 4, name);			
		snprintf(key, 5, KEY_FORMAT_FAN_SPEED, no);
		fanSensor[card_number]=new FanSensor(key, TYPE_FPE2, 2);
		UpdateFNum();
	}
}

#endif
// this is a sample from linux

/* get temperature in millidegrees */
static UInt32 rv770_get_temp()
{
	UInt32 temp = (INVID(CG_MULT_THERMAL_STATUS) & ASIC_TM_MASK) >>
	ASIC_TM_SHIFT;
	UInt32 actual_temp = 0;
	
	if ((temp >> 9) & 1)
		actual_temp = 0;
	else
		actual_temp = (temp >> 1) & 0xff;
	
	return actual_temp * 1000;
}
static UInt32 rv6xx_get_temp()
{
	UInt32 temp = (INVID(CG_THERMAL_STATUS) & ASIC_T_MASK) >>
	ASIC_T_SHIFT;
	UInt32 actual_temp = 0;
	
	if ((temp >> 7) & 1)
		actual_temp = 0;
	else
		actual_temp = (temp >> 1) & 0xff;
	
	return actual_temp * 1000;
}

void RadeonPlugin::getRadeonInfo()
{
	UInt16 devID = chipID >> 16;
	for (int i=0; radeon_device_list[i].device_id; i++) {
		if (devID == radeon_device_list[i].device_id) {
			rinfo = &radeon_device_list[i];
			break;
		}
	}
}

void RadeonPlugin::setup_R6xx(int card_number)
{
	char key[5];
	snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card_number);
	tempSensor[card_number]=new TemperatureSensor(key, TYPE_SP78, 2);
	Caps = GPU_TEMP_MONITORING;
	getTemp = rv6xx_get_temp;
		
}

void RadeonPlugin::setup_R7xx(int card_number)
{
	char key[5];
	snprintf(key, 5, KEY_FORMAT_GPU_DIODE_TEMPERATURE, card_number);
	tempSensor[card_number]=new TemperatureSensor(key, TYPE_SP78, 2);
	Caps = GPU_TEMP_MONITORING;
	getTemp = rv770_get_temp;
}


bool RadeonPlugin::init(OSDictionary *properties)
{    
	return super::init(properties);
}

void RadeonPlugin::stop (IOService* provider)
{
	for (int card_number=0; card_number<max_card; card_number++) {
		if(tempSensor[card_number])
			delete tempSensor[card_number];
		if (boardSensor[card_number])
			delete boardSensor[card_number];
		if (fanSensor[card_number]) {
			delete fanSensor[card_number];
		}
	}
	UpdateFNum();
	super::stop(provider);
}

void RadeonPlugin::free ()
{
	super::free ();
}

IOReturn TemperatureSensor::OnKeyRead(const char* key, char* data)
{
	int i = key[2] - '0';
	if(!i){  // for a single card
		return kIOReturnSuccess;
	}
	switch(key[3]){
		case 'D':
			if(RadeonService->Caps & (GPU_TEMP_MONITORING)){
				data[0]=get_gpu_temp(RadeonService->tempSensor[i]);
				data[1]=0;
				return kIOReturnSuccess;
			}
			break;
		case 'H':
			if(RadeonService->caps & (BOARD_TEMP_MONITORING)) {
				data[0]=get_board_temp(RadeonService->boardSensor[1]);
				data[1]=0;
				return kIOReturnSuccess;
			}
	}
	IOLog("Error: temperature monitoring %s isn't supported on your videocard.\n", key);
	return kIOReturnSuccess;
}

IOReturn FanSensor::OnKeyRead(const char* key, char* data)
{
	int i = key[2] - '0';
	if(!i){  // for a single card
		return kIOReturnSuccess;
	}
	
	if(Caps & I2C_FANSPEED_MONITORING)
	{
		UInt16 rpm=get_i2c_fanspeed_rpm(fanSensor[i]);
		data[0]=(rpm<<2)>>8;
		data[1]=(rpm<<2)&0xff;
	}
	else if(nv_card->caps & GPU_FANSPEED_MONITORING)
	{
		UInt16 rpm=get_fanspeed(fanSensor[i]);
		data[0]=(rpm<<2)>>8;
		data[1]=(rpm<<2)&0xff;
	}
	return  kIOReturnSuccess;
}