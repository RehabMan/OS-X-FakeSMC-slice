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

void FintekTemperatureSensor::OnKeyRead(__unused const char* key, char* data)
{
	data[0] = Fintek_ReadTemperature(m_Address, m_Model, m_Offset);
	data[1] = 0;
}

void FintekTemperatureSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void FintekVoltageSensor::OnKeyRead(__unused const char* key, char* data)
{
	UInt16 value = Fintek_ReadByte(m_Address, FINTEK_VOLTAGE_BASE_REG + m_Offset) << 4;
	float V = (m_Offset == 1 ? 0.5f : 1.0f) * 0.001f * value;
	
	value = fp2e_Encode(V);
	
	data[0] = (value & 0xff00) >> 8;
	data[1] = value & 0x00ff;
}

void FintekVoltageSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void FintekTachometerSensor::OnKeyRead(__unused const char* key, char* data)
{
	int value = Fintek_ReadTachometer(m_Address, m_Offset);
		
	if (value > 0)
	{
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	}
}

void FintekTachometerSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void Fintek::Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x87);
}

void Fintek::Exit()
{
	outb(RegisterPort, 0xAA);
}

bool Fintek::Probe()
{
	Model = UnknownModel;
	
	for (int i = 0; i < FINTEK_PORTS_COUNT; i++) 
	{
		RegisterPort	= FINTEK_PORT[i];
		ValuePort		= FINTEK_PORT[i] + 1;
		
		Enter();
		
		UInt8 logicalDeviceNumber = 0;
		
        UInt8 id = Fintek_ReadByte(Address, FINTEK_CHIP_ID_REGISTER);
        UInt8 revision = Fintek_ReadByte(Address, FINTEK_CHIP_REVISION_REGISTER);
        
        switch (id) 
		{
			case 0x05:
			{
				switch (revision) 
				{
					case 0x07:
						Model = F71858;
						logicalDeviceNumber = F71858_HARDWARE_MONITOR_LDN;
						break;
					case 0x41:
						Model = F71882;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				}
			} break;
			case 0x06:
			{
				switch (revision) 
				{
					case 0x01:
						Model = F71862;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				} 
			} break;
			case 0x07:
			{
				switch (revision)
				{
					case 0x23:
						Model = F71889F;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				} 
			} break;
			case 0x08:
			{
				switch (revision)
				{
					case 0x14:
						Model = F71869;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				}
			} break;
			case 0x09:
			{
				switch (revision)
				{
					case 0x09:
						Model = F71889ED;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				}
			} break;
		}
		
		Select(logicalDeviceNumber);
		
		Address = ReadWord(SUPERIO_BASE_ADDRESS_REGISTER);          
		
		IOSleep(1000);
		
		UInt16 verify = ReadWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
		Exit();
		
		if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
			continue;
		
		if (Model == UnknownModel)
		{
			InfoLog("found unsupported Fintek chip ID=0x%x REVISION=0x%x on ADDRESS=0x%x", id, revision, Address);
			continue;
		} 
		else
		{
			return true;
		}
	}
	
	return false;
}

void Fintek::Init()
{
	// Heatsink
	Bind(new FintekTemperatureSensor(Address, Model, 0, "Th0H", "sp78", 2));
	// Northbridge
	Bind(new FintekTemperatureSensor(Address, Model, 1, "TN0P", "sp78", 2));
	
	switch (Model) 
	{
        case F71858:
			break;
        default:
			// CPU Vcore
			Bind(new FintekVoltageSensor(Address, 1, "VC0C", "fp2e", 2));
			break;
	}
	
	FanOffset = GetFNum();
	
	for (int i = 0; i < (Model == F71882 ? 4 : 3); i++) 
	{
		char key[5];
		bool fanName = FanName[i] && strlen(FanName[i]) > 0;
		
		if (fanName || Fintek_ReadTachometer(Address, i) > 0)
		{	
			if(fanName)
			{
				snprintf(key, 5, "F%dID", FanOffset + FanCount);
				FakeSMCAddKey(key, "ch8*", strlen(FanName[i]), (char*)FanName[i]);
			}
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			Bind(new FintekTachometerSensor(Address, i, key, "fpe2", 2));
			
			FanIndex[FanCount++] = i;
		}
	}
	
	UpdateFNum(FanCount);
}

void Fintek::Finish()
{
	FlushBindings();
	UpdateFNum(-FanCount);
}
