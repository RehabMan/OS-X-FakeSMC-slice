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
	outb(Address + FINTEK_ADDRESS_REGISTER_OFFSET, reg);
	return inb(Address + FINTEK_DATA_REGISTER_OFFSET);
} 

SInt16 Fintek::ReadTemperature(UInt8 index)
{
	float value;
	
	switch (Model) 
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
	UInt16 value = Fintek::ReadByte(FINTEK_VOLTAGE_BASE_REG + index) << 4;
	float V = (index == 1 ? 0.5f : 1.0f) * 0.001f * value;

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
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x87);
}

void Fintek::Exit()
{
	outb(RegisterPort, 0xAA);
}

bool Fintek::Probe()
{
	DebugLog("Probing Fintek...");
	
	Model = UnknownModel;
	
	for (int i = 0; i < FINTEK_PORTS_COUNT; i++) 
	{
		RegisterPort	= FINTEK_PORT[i];
		ValuePort		= FINTEK_PORT[i] + 1;
		
		Enter();
		
		UInt8 logicalDeviceNumber = 0;
		
        UInt8 id = ListenPortByte(FINTEK_CHIP_ID_REGISTER);
        UInt8 revision = ListenPortByte(FINTEK_CHIP_REVISION_REGISTER);
        
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
		
		Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);          
		
		IOSleep(1000);
		
		UInt16 verify = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
		Exit();
		
		if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
			continue;
		
		if (Model == UnknownModel)
		{
			InfoLog("Found unsupported Fintek chip ID=0x%x REVISION=0x%x on ADDRESS=0x%x", id, revision, Address);
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
	Bind(new FintekTemperatureSensor(this, 0, "Th0H", "sp78", 2));
	// Northbridge
	Bind(new FintekTemperatureSensor(this, 1, "TN0P", "sp78", 2));
	
	switch (Model) 
	{
        case F71858:
			break;
        default:
			// CPU Vcore
			Bind(new FintekVoltageSensor(this, 1, "VC0C", "fp2e", 2));
			break;
	}
	
	FanOffset = GetFNum();
	
	for (int i = 0; i < (Model == F71882 ? 4 : 3); i++) 
	{
		char key[5];
		bool fanName = FanName[i] && strlen(FanName[i]) > 0;
		
		if (fanName || ReadTachometer(i) > 0)
		{	
			if(fanName)
			{
				snprintf(key, 5, "F%dID", FanOffset + FanCount);
				FakeSMCAddKey(key, "ch8*", strlen(FanName[i]), (char*)FanName[i]);
			}
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			Bind(new FintekTachometerSensor(this, i, key, "fpe2", 2));
			
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
