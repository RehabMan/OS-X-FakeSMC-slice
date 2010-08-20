/*
 *  SMSC.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 22/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

//Slice - some comments from datasheets
#if 0
-----------------------------------------------------------
switch (id) {
	case 0x5a: /* LPC47N227 */
		runtime_base = regval(port, 0x30) << 4;
		if (runtime_base)
			dump_io(runtime_base, 16);
			else
				printf("Runtime Register Block not mapped on this Super I/O.\n");
				break;
	default:
		printf("No extra registers known for this chip.\n");
}
--------------------------------------------------------------
SMSC LDNs
0 Floppy
1 PM1
2 COM2 (47M182)
3 LPT
4 COM1
5 COM2
6 RTC
7 Keyboard
8 EC
9 Mailbox 
A LPC/8051 GPIO (FAN control), PME, Runtime Registers
B MIDI port (MPU-401), SMBus
----------------------------------------------------------------
Fan Tachometer Register ( 1 =0x59, 2 =0x5A LDN=0x0A)
Bit[7:0] The 8-bit FAN1 tachometer count. The number of counts of the internal clock per pulse of the fan. The count value is computed from Equation 1. This value is the final (maximum) count of the previous pulse (latched). The value in this register may not be valid for up to 2 pulses following a write to the preload register.
----------------------------------------------------------------
Fan Control Register (0x58 LDN=0x0A)
Bit[0] Fan 1 Clock Source Select
This bit and the Fan 1 Clock Multiplier bit is used with The Fan 1 Clock Select bit in the Fan 1 register (0x56) to determine the fan speed FOUT.	See Table 56 in “Fan Speed Control and Monitoring” section.
Bit[1] Fan 2 Clock Source Select This bit and the Fan 2 Clock Multiplier bit is used with The Fan 2 Clock Select bit in the Fan 2 register (0x57) to determine the fan speed FOUT.	See Table 56 in “Fan Speed Control and Monitoring” section. Bit[2] Fan 1 Clock multiplier
0=No multiplier used
1=Double the fan speed selected by bit 0 of this register and bit 7 of the FAN1 register
Bit[3] Fan 2 Clock multiplier 0=No multiplier used
1=Double the fan speed selected by bit 1 of this register and bit 7 of the FAN2 register
Bit[5:4] The FAN1 count divisor. Clock scalar for adjusting the tachometer count. Default = 2.
00: divisor = 1 01: divisor = 2 10: divisor = 4 11: divisor = 8
Bit[7:6] The FAN2 count divisor. Clock scalar for adjusting the tachometer count. Default = 2.
00: divisor = 1 01: divisor = 2 10: divisor = 4 11: divisor = 8
-----------------------------------------------------------------------------


#endif  //datasheets

#include "SMSC.h"

void SMSC::Enter()
{
	outb(m_RegisterPort, 0x55);
	outb(m_RegisterPort, 0x55);
}

void SMSC::Exit()
{
	outb(m_RegisterPort, 0xAA);
	//Some sort of reset chip to default mode or like this
	outb(m_RegisterPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(m_ValuePort, 0x02);
}

UInt8 SMSC::ReadByte(/*UInt8 ldn,*/ UInt8 reg) 
{
	// Not needed cose we already selected FAN device
	//outb(m_RegisterPort, SUPERIO_DEVICE_SELECT_REGISTER);
	//outb(m_ValuePort, ldn);
	outb(m_RegisterPort, reg);
	return inb(m_ValuePort);
}

void SMSC::WriteByte(/*UInt8 ldn,*/ UInt8 reg, UInt8 value)
{
	//outb(m_RegisterPort, SUPERIO_DEVICE_SELECT_REGISTER);
	//outb(m_ValuePort, ldn);
	outb(m_RegisterPort, reg);
	outb(m_ValuePort, value); 
}

SInt16 SMSC::ReadTachometer(__unused UInt8 index)
{
	return 0;
}

bool SMSC::ProbePort()
{
	UInt16 id = ListenPortByte(SUPERIO_CHIP_ID_REGISTER);
		
	if (id != 0 && id != 0xffff)
		return false;
	
	UInt8 logicalDeviceNumber = SMSC_HARDWARE_MONITOR_LDN;
	
	m_FanLimit = 2;
	
	switch (id >> 8) 
	{
		case LPC47B397_NC:
		case SCH4307:
		case SCH5317_1:
		case SCH5317_2:
			logicalDeviceNumber = SMSC_HARDWARE_MONITOR_LDN_VA;
			m_Model = id >> 8;
			break;
			
		case LPC47N252:
			logicalDeviceNumber = LPC47N252_HARDWARE_MONITOR_LDN;
			m_Model = id >> 8;
			break;
			
		case LPC47B27x: 
		case LPC47B37x:
		case LPC47M10x_112_13x:
		case LPC47M14x:
		case LPC47M15x_192_997:
		case LPC47M172:
		case LPC47M182:
		case LPC47S42x:
		case LPC47S45x:
		case LPC47U33x:
		case SCH3112:
		case SCH3114:
		case SCH3116:		
		case SCH5127:
		case SCH5307_NS:
			m_Model = id >> 8;
			break;
			
		default:
		{
			switch (id)
			{
				case LPC47M233:
				case LPC47M292:
					m_Model = id;
					break;
					
				default:
					InfoLog("SMSC: Found unsupported chip ID=0x%x", id);
					return false;
			}
		} break;
	}
	
	Select(logicalDeviceNumber);
	
	// Getting address of FAN or HWMonitor logical device
	m_Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);          
	
	IOSleep(1000);
	
	UInt16 verify = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
	
	if (m_Address != verify || m_Address < 0x100 || (m_Address & 0xF007) != 0)
		return false;
	
	WarningLog("SMSC sensors monitoring is not supported now");
#if 1
	//Slice - registers dump for development purpose
	IOLog("SMSC ldn=10 registers dump\n");
	for (UInt16 i=0x56; i<0x62; i++) {
		IOLog("%02x:%04x ", i, ListenPortByte(i));
	}
	IOLog("\n");
#endif
	
	return true;
}

void SMSC::Start()
{
	
}