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

void NSC::Enter()
{
}

void NSC::Exit()
{
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

SInt16 NSC::ReadTemperature(__unused UInt8 index)
{
	
	float temperature = 25.0f;
	
	return temperature;
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
	
	
		
		Select(NSC_HARDWARE_MONITOR_LDN);
		
		m_Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);          
		
		IOSleep(1000);
		
//		UInt16 verify = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
		//Exit();
//Slice -- not sure if it is correct for NSC		
//		if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
//			continue;
		
		
		if (m_Model == UnknownModel)
		{
			InfoLog("found unsupported NSC chip ID=0x%x REVISION=0x%x on ADDRESS=0x%x", id, revision, m_Address);
#if 1
			//Slice - registers dump for development purpose
			IOLog("NSC ldn=15 registers dump\n");
			for (UInt16 k=0x0; k<0xff; k+=16) {
				IOLog("%02x: ", k);
				for (UInt16 j=0; j<16; j++) {
					IOLog("%02x ", ListenPortByte(k+j));
				}
				IOLog("\n");
			}
			
#endif
			
			return false;
		} 
		else
		{
			return true;
		}
	
	return false;
}

void NSC::Start()
{
	
}

#if 0
void NSC::Finish()
{
//	FlushBindings();
//	UpdateFNum(-FanCount);
}
#endif