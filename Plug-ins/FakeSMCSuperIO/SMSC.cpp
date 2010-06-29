/*
 *  SMSC.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 22/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "SMSC.h"

void SMSC::Enter()
{
	outb(m_RegisterPort, 0x55);
	outb(m_RegisterPort, 0x55);
}

void SMSC::Exit()
{
	outb(m_RegisterPort, 0xAA);
	outb(m_RegisterPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(m_ValuePort, 0x02);
}

bool SMSC::ProbePort()
{
	UInt16 id = ListenPortByte(SUPERIO_CHIP_ID_REGISTER);
		
	if (id != 0 && id != 0xffff)
		return false;
	
	UInt8 logicalDeviceNumber = SMSC_HARDWARE_MONITOR_LDN;
	
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
					InfoLog("Found unsupported SMSC chip ID=0x%x", id);
					return false;
			}
		} break;
	}
	
	Select(logicalDeviceNumber);
	
	m_Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);          
	
	IOSleep(1000);
	
	UInt16 verify = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
	
	if (m_Address != verify || m_Address < 0x100 || (m_Address & 0xF007) != 0)
		return false;
	
	WarningLog("SMSC sensors monitoring is not supported now");
	
	return true;
}

void SMSC::Start()
{
	
}