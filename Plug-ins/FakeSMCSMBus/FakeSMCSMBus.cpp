/* 
 *  FakeSMC x3100 GPU temperature plugin
 *  Created by Slice 28.05.2010
 *  Copyright 2010 Slice. All rights reserved.
 *
 */
#include "FakeSMCSMBus.h"

#define kMCHBAR 0x10
#define INVID8(offset) (mmio_base[offset])
#define INVID16(offset) OSReadLittleInt16((mmio_base), offset)
#define INVID(offset) OSReadLittleInt32((mmio_base), offset)
#define OUTVID16(offset,val) OSWriteLittleInt16((mmio_base), offset, val)
#define OUTVID(offset,val) OSWriteLittleInt32((mmio_base), offset, val)

OSDefineMetaClassAndStructors(SMBplugin, IOService) 

SMBplugin * SMBdevice;

#define super IOService
static void Update(const char* key, char* data) {	
	if(CompareKeys(key, "TG0D"))
	{
		data[0] = SMBdevice->update(0);
		data[1] = 0;
	} else  if(CompareKeys(key, "TG0P"))
	{		
		data[0] = SMBdevice->update(1);
		data[1] = 0;
	}
}


int SMBplugin::update(int keyN)
{	
	UInt16 value = 0;  //unused
	if (I2Csensor->ReadData16(keyN, I2Csensor->addr[0], I2Csensor->command[0], &value)) {
		return (int)value;
	} 
	return 0;
}

IOService*
SMBplugin::probe(IOService *provider, SInt32 *score)
{
#if DEBUG	
	OSData*		prop;
	UInt32		Vchip;
	
	prop = OSDynamicCast( OSData , provider->getProperty(fVendor)); // safe way to get vendor
	if(prop)
	{
		Vchip = *(UInt32*) prop->getBytesNoCopy();
		IOLog("FakeSMC_SMBus: found %lx chip\n", (long unsigned int)Vchip);
		if( (Vchip & 0xffff) != 0x1002) //check if vendorID is really AMD, if not don't bother
		{
			//IOLog("Can't Find AMD Chip!\n");
			return( 0 );
		}		
	}
#endif	
    if( !super::probe( provider, score ))
		return( 0 );
	//	IOLog("FakeSMC_SMBus: probe success\n");	
	return (this);
}

bool
SMBplugin::start( IOService * provider ) {
	if(!provider || !super::start(provider))
		return false;
	//	OSData*		idKey;	
	//	IOLog("FakeSMC_SMBus: starting\n");	
	
	
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
	//		IOLog("FakeSMC_SMBus: register space=%08lx\n", (long unsigned int)bar);
	theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x2000, kIODirectionOutIn); // | kIOMapInhibitCache);
	if(theDescriptor != NULL)
	{
		mmio = theDescriptor->map ();
		if(mmio != NULL)
		{
			//		UInt32 addr = map->getPhysicalAddress();
			mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
#if 1				
			IOLog("FakeSMC_SMBus: MCHBAR mapped\n");
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
			IOLog("FakeSMC_SMBus: MCHBAR failed to map\n");
			return -1;
		}			
	}
	
	// Now setup I2C
	I2Csensor = new I2Cline;
	if (I2Csensor) {
		I2Csensor->Mode = PCI_mode;
		I2Csensor->I2Cdevice = VCard;
		//	I2Csensor->mmio_base = mmio_base;  // no needed for this case
		
		/*
		 * This is example project for I2C bus sensors
		 * good description with AMD SB700 chipset
		 * SMBus controller PCIClass = 0x0c050000
		 * GPIO_52_to_49_Cntrl - RW – 16 bits - [PCI_Reg: 50h]
		 Field Name		Bits	Default
		 
		 GPIO_Out		3:0		0h
		 GPIO_Out_En#	7:4		Fh
		 GPIO_Status	11:8
		 Reserved -		15:12	0h
		 
		 Description
		 Write 1 to set and 0 to clear each of the GPIO port;
		 providing the corresponding enable bits (7:4) are set to 0
		 Bit [0] for GPIO49/FANOUT2
		 Bit [1] for GPIO50/FANIN0
		 Bit [2] for GPIO51/FANIN1 
		 Bit [3] for GPIO52/FANIN2
		 GPIO output port enable for each of the GPIO port
		 0:		Output = GPIO_Out
		 1:		Output = tristate
		 GPIO input status for each of the GPIO port
		 *
		 * The same for reg 0x56 temp0-3 GPIO_64_to_61_Cntrl - RW – 16 bits - [PCI_Reg: 56h]
		 
		 */
		I2Cdefines * p = &I2Csensor->I2C[0]; // sensor Nr 0 suppose it is temp 1 (bits 1, 5)
		{
			p->n_out_clk_reg = 0;
			p->n_out_data_reg = 0; //the same
			p->in_clk_reg = 0x56;
			p->in_data_reg = 0x56;
			p->out_clk_reg = 0x56;
			p->out_data_reg = 0x56;
			p->clk_mask = 1<<5;     // GPIO_EN bit == clock
			p->clk_sign = 0;		// enable=0 tristate=1
			p->data_mask = 1<<1;	// GPIO_OUT bit for temp1
			p->data_sign = 1;		// data=1 means high
		//timings from SMBus datasheet nanoseconds
			p->t_out = 25000;
			p->t_low = 4700;
			p->t_high = 4000;
			p->t_buf = 4700;
			p->tsu_dat = 250;
			p->thd_dat = 300;
			p->tsu_sta = 4700;
			p->thd_sta = 4000;
			p->tsu_sto = 4000;
		}
		I2Csensor->oldvalues[0] = 0xf << 4;  //default value from SB700
		I2Csensor->addr[0] = 0x12;  //subaddr from MAXIM datasheet
		I2Csensor->command[0] = 0x31; // no any idea
	}
	
	char value[2];
	value[0] = 10;
	value[1] = 0;
	FakeSMCAddKeyCallback("TG0D", "sp78", 2, value, &Update);
	
	IOLog("FakeSMC_SMBus: keys TG0D registered\n");	
	SMBdevice = this;
	return true;	
	
}
