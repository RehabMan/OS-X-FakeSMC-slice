/*
 *  NSC.cpp
 *  FakeSMC plugin to monitor SuperIO by National Semiconductors
 *  NSC part written by Slice 24/06/2010
 *
 *  Original SuperIO created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#include "NSC.h"
#include "NSCsensor.h"

void NSC::Enter()
{
	//Not needed
}

void NSC::Exit()
{
	//Not needed
}

UInt8 NSC::ReadByte(__unused UInt8 bank, UInt8 reg) 
{
	// Not needed cose we already selected FAN device
	//outb(m_RegisterPort, SUPERIO_DEVICE_SELECT_REGISTER);
	//outb(m_ValuePort, ldn);
	outb(m_RegisterPort, reg);
	return inb(m_ValuePort);
}

void NSC::WriteByte(__unused UInt8 bank, UInt8 reg, UInt8 value)
{
	//outb(m_RegisterPort, SUPERIO_DEVICE_SELECT_REGISTER);
	//outb(m_ValuePort, ldn);
	outb(m_RegisterPort, reg);
	outb(m_ValuePort, value); 
}

SInt16 NSC::ReadVoltage(__unused UInt8 index)
{
	float voltage = 1.0f; 
	return voltage;
}


bool NSC::ProbePort()
{	
//	DebugLog("Probing NSC...");
	UInt16 id = ListenPortByte(SUPERIO_CHIP_ID_REGISTER);	
	UInt8 revision = ListenPortByte(NSC_CHIP_REVISION_REGISTER);
	if (id == 0 || id == 0xff || revision == 0 || revision == 0xff)
		return false;
	if ((id == 0xfc) /* && (revision == 0x66)*/) {
		Select(NSC_HARDWARE_MONITOR_LDN);
		
		m_Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER); 
		if (!ListenPortByte(NSC_LDN_PRESENT)) {
			return false;
		}
		
		InfoLog("found supported NSC chip ID=0x%x REVISION=0x%x on ADDRESS=0x%x", id, revision, m_Address);
		m_Model = PC8739x;
		return true;

	}
		else
	{
//			InfoLog("found unsupported NSC chip ID=0x%x REVISION=0x%x on ADDRESS=0x%x", id, revision, m_Address);
#if 0
			//Slice - registers dump for development purpose
			IOLog("NSC ldn=15 registers dump\n");
			for (UInt16 k=0x0; k<0xff; k+=16) {
				IOLog("%02x: ", k);
				for (UInt16 j=0; j<16; j++) {
					IOLog("%02x ", ListenPortByte(k+j));
				}
				IOLog("\n");
			}
			return true; //Slice - temporarily
			
#endif
			
		return false;
	} 
}

void NSC::Start()
{
	IOMemoryDescriptor *		theDescriptor;
	UInt32 adr = (ListenPortByte(NSC_MEM)&0xff)+((ListenPortByte(NSC_MEM+1)<<8)&0xff00)+
		((ListenPortByte(NSC_MEM+2)&0xff)<<16)+((ListenPortByte(NSC_MEM+3)&0xff)<<24);
	
	IOPhysicalAddress bar = (IOPhysicalAddress)(adr & ~0xf);
	//		IOLog("Fx3100: register space=%08lx\n", (long unsigned int)bar);
	theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x200, kIODirectionOutIn); // | kIOMapInhibitCache);
	if(theDescriptor != NULL)
	{
		mmio = theDescriptor->map ();
		if(mmio != NULL)
		{
			//		UInt32 addr = map->getPhysicalAddress();
			mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
#if 0				
			UInt32 base_phys = (UInt32)mmio->getPhysicalAddress();
			InfoLog(" Memory mapped at address %08lx\n", (long unsigned int)base_phys);
			for (int i=0; i<0x2f; i +=16) {
				IOLog("%04lx: ", (long unsigned int)i);
				for (int j=0; j<16; j += 1) {
					IOLog("%02lx ", (long unsigned int)mmio_base[i+j]);
				}
				IOLog("\n");
			}
			//mmio->release();
#endif				
		}
		else
		{
			InfoLog(" MCHBAR failed to map\n");
			return;
		}			
	}	
	
	
	// Heatsink
	AddSensor(new NSCTemperatureSensor(this, 0, KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2));
	// Northbridge
	AddSensor(new NSCTemperatureSensor(this, 1, KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2));
	// Heatsink
	AddSensor(new NSCTemperatureSensor(this, 2, KEY_DIMM_TEMPERATURE, TYPE_SP78, 2));
	// Northbridge
	AddSensor(new NSCTemperatureSensor(this, 3, KEY_AUX_TEMPERATURE, TYPE_SP78, 2));

	char key[5];
	int id=GetNextUnusedKey(KEY_FORMAT_FAN_ID, key);
	int ac=GetNextUnusedKey(KEY_FORMAT_FAN_SPEED, key);
	if (id!=-1 || ac!=-1) {
		int no=id>ac ? id : ac;
		char name[] = "System Fan"; 
		int lname = sizeof(name);
//		snprintf (name, 10, "System Fan");
		snprintf(key, 5, KEY_FORMAT_FAN_ID, no);
		FakeSMCAddKey(key, TYPE_CH8, lname, name);			
		snprintf(key, 5, KEY_FORMAT_FAN_SPEED, no);
		AddSensor(new NSCTachometerSensor(this, 3, key, TYPE_FP2E, 2));
		UpdateFNum();
		FanCount = 1;
	}	
	
}
		
		
		void NSC::Finish()
{
//	FlushBindings();
	UpdateFNum();
}

SInt16	NSC::ReadTemperature(UInt8 index)
{
	int offset = NSC_HARDWARE_MONITOR_REGS[index];
	return mmio_base[offset];
}

SInt16	NSC::ReadTachometer(UInt8 index)
{
	int offset = NSC_HARDWARE_MONITOR_REGS[index];
	UInt8 speed = ~mmio_base[offset];
	return speed*10;
}
		
