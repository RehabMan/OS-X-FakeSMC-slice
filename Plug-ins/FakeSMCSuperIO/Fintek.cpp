/*
 *  Fintek.cpp
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 31/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#include "Fintek.h"
#include "FintekSensors.h"

UInt8 Fintek::ReadByte(UInt8 reg) 
{
	outb(m_Address + FINTEK_ADDRESS_REGISTER_OFFSET, reg);
	return inb(m_Address + FINTEK_DATA_REGISTER_OFFSET);
} 

SInt16 Fintek::ReadTemperature(UInt8 index)
{
	float value;
	
	switch (m_Model) 
	{
		case F71858: 
		{
			int tableMode = 0x3 & Fintek::ReadByte(FINTEK_TEMPERATURE_CONFIG_REG);
			int high = Fintek::ReadByte(FINTEK_TEMPERATURE_BASE_REG + 2 * index);
			int low = Fintek::ReadByte(FINTEK_TEMPERATURE_BASE_REG + 2 * index + 1);      
			
			if (high != 0xbb && high != 0xcc) 
			{
                int bits = 0;
				
                switch (tableMode) 
				{
					case 0: bits = 0; break;
					case 1: bits = 0; break;
					case 2: bits = (high & 0x80) << 8; break;
					case 3: bits = (low & 0x01) << 15; break;
                }
                bits |= high << 7;
                bits |= (low & 0xe0) >> 1;
				
                short value = (short)(bits & 0xfff0);
				
				return (float)value / 128.0f;
			} 
			else 
			{
                return 0;
			}
		} break;
		default: 
		{
            value = Fintek::ReadByte(FINTEK_TEMPERATURE_BASE_REG + 2 * (index + 1));
		} break;
	}
	
	return value;
}

SInt16 Fintek::ReadVoltage(UInt8 index)
{
	UInt16 raw = Fintek::ReadByte(FINTEK_VOLTAGE_BASE_REG + index);
	
	if (index == 0) m_RawVCore = raw;
	
	float V = (index == 1 ? 0.5f : 1.0f) * 0.001f * (raw << 4);
	
	return V;
}

SInt16 Fintek::ReadTachometer(UInt8 index)
{
	int value = Fintek::ReadByte(FINTEK_FAN_TACHOMETER_REG[index]) << 8;
	value |= Fintek::ReadByte(FINTEK_FAN_TACHOMETER_REG[index] + 1);
	
	if (value > 0)
		value = (value < 0x0fff) ? 1.5e6f / value : 0;
	
	return value;
}


void Fintek::Enter()
{
	outb(m_RegisterPort, 0x87);
	outb(m_RegisterPort, 0x87);
}

void Fintek::Exit()
{
	outb(m_RegisterPort, 0xAA);
	outb(m_RegisterPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(m_ValuePort, 0x02);
}

bool Fintek::ProbePort()
{
	UInt8 logicalDeviceNumber = 0;
	
	UInt8 id = ListenPortByte(FINTEK_CHIP_ID_REGISTER);
	UInt8 revision = ListenPortByte(FINTEK_CHIP_REVISION_REGISTER);
	
	if (id == 0 || id == 0xff || revision == 0 || revision == 0xff)
		return false;
	
	switch (id) 
	{
		case 0x05:
		{
			switch (revision) 
			{
				case 0x07:
					m_Model = F71858;
					logicalDeviceNumber = F71858_HARDWARE_MONITOR_LDN;
					break;
				case 0x41:
					m_Model = F71882;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			}
		} break;
		case 0x06:
		{
			switch (revision) 
			{
				case 0x01:
					m_Model = F71862;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			} 
		} break;
		case 0x07:
		{
			switch (revision)
			{
				case 0x23:
					m_Model = F71889F;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			} 
		} break;
		case 0x08:
		{
			switch (revision)
			{
				case 0x14:
					m_Model = F71869;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			}
		} break;
		case 0x09:
		{
			switch (revision)
			{
				case 0x09:
					m_Model = F71889ED;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			}
		} break;
	}
	
	if (m_Model == UnknownModel)
	{
		InfoLog("Found unsupported Fintek chip ID=0x%x REVISION=0x%x", id, revision);
		return false;
	} 
	
	Select(logicalDeviceNumber);
	
	m_Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);          
	
	IOSleep(1000);
	
	UInt16 verify = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
	
	if (m_Address != verify || m_Address < 0x100 || (m_Address & 0xF007) != 0)
		return false;
	
	return true;
}

void Fintek::Start()
{
	// Heatsink
	AddBinding(new FintekTemperatureSensor(this, 0, KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2));
	// Northbridge
	AddBinding(new FintekTemperatureSensor(this, 1, KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2));
	
	switch (m_Model) 
	{
        case F71858:
			break;
        default:
			// CPU Vcore
			AddBinding(new FintekVoltageSensor(this, 1, KEY_CPU_VOLTAGE, TYPE_FP2E, 2));
			break;
	}
	
	// FANs
	for (int i = 0; i < (m_Model == F71882 ? 4 : 3); i++) 
	{
		char* key = (char*)IOMalloc(5);
		
		bool fanName = m_FanName[i] && strlen(m_FanName[i]) > 0;
		
		if (fanName || ReadTachometer(i) > 0)
		{	
			int offset = GetNextFanIndex();
			
			if (offset != -1) 
			{
				if (fanName)
				{
					snprintf(key, 5, KEY_FORMAT_FAN_ID, offset); 
					FakeSMCAddKey(key, TYPE_CH8, strlen(m_FanName[i]), (char*)m_FanName[i]);
					
					snprintf(key, 5, KEY_FORMAT_FAN_SPEED, offset); 
					AddBinding(new FintekTachometerSensor(this, i, key, TYPE_FPE2, 2));
					
					m_FanIndex[m_FanCount++] = i;
				}
				
				snprintf(key, 5, KEY_FORMAT_FAN_SPEED, offset); 
				AddBinding(new FintekTachometerSensor(this, i, key, TYPE_FPE2, 2));
				
				m_FanIndex[m_FanCount++] = i;
			}
		}
		
		IOFree(key, 5);
	}
	
	UpdateFNum();
}
