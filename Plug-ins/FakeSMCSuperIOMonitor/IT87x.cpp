/*
 *  IT87x.cpp
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include "IT87x.h"
#include "fakesmc.h"

void IT87x::SetPorts(UInt8 index)
{
	RegisterPort = ITE_PORT[index];
	ValuePort = ITE_PORT[index] + 1;
}

void IT87x::Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x01);
	outb(RegisterPort, 0x55);
	
	if (RegisterPort == 0x4e) 
	{
		outb(RegisterPort, 0xaa);
	}
	else
	{
		outb(RegisterPort, 0x55);
	}
}

void IT87x::Exit()
{
	outb(RegisterPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(ValuePort, 0x02);
}

UInt8 IT87x::ReadByte(UInt8 reg, bool* valid)
{
	outb(Address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	UInt8 value = inb(Address + ITE_DATA_REGISTER_OFFSET);
	
	valid = (bool*)(reg == inb(Address + ITE_DATA_REGISTER_OFFSET));
	
	return value;
}

UInt16 IT87x::ReadRPM(UInt8 num)
{
	bool* valid;
	int value = ReadByte(ITE_FAN_TACHOMETER_REG[num], valid);
	
	if(valid)
	{
		value |= ReadByte(ITE_FAN_TACHOMETER_EXT_REG[num], valid) << 8;
		value = valid && value > 0x3f && value < 0xffff ? (float)(1350000 + value) / (float)(value * 2) : 0;
	}
	
	return value;
}

bool IT87x::Probe()
{
	Model = UnknownModel;
	
	for (int i = 0; i < ITE_PORTS_COUNT; i++) 
	{
		SetPorts(i);
		
		Enter();
		
		UInt16 chipID = ReadWord(SUPERIO_CHIP_ID_REGISTER);
		
		switch (chipID)
		{
			case IT8712F:
				Model = IT8712F; 
				break;
			case IT8716F:
				Model = IT8716F; 
				break;
			case IT8718F:
				Model = IT8718F; 
				break;
			case IT8720F: 
				Model = IT8720F; 
				break;
			case IT8726F: 
				Model = IT8726F; 
				break; 
			default: 
				Model = UnknownModel;
				break;
		}
		
		Select(IT87_ENVIRONMENT_CONTROLLER_LDN);
		
		Address = ReadWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
		IOSleep(1000);
		
		UInt16 verify = ReadWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
		Exit();
		
		if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
			continue;
		
		bool* valid;
		UInt8 vendorId;
		
		vendorId = ReadByte(ITE_VENDOR_ID_REGISTER, valid);
		
		if (!valid || vendorId != ITE_VENDOR_ID)
			continue;
		
		if ((ReadByte(ITE_CONFIGURATION_REGISTER, valid) & 0x10) == 0)
			continue;
		
		if (!valid)
			continue;
		
		if (Model == UnknownModel)
		{
			InfoLog("found unsupported ITE chip ID=0x%x on ADDRESS=0x%x", chipID, Address);
			continue;
		} 
		else
		{		
			return true;			
		}
	}
	
	return false;
}

void IT87x::Init()
{
	char value[2];
	
	Instance = this;
	
	// Temperature semi-autodetection
	
	int count = 0;
	
	for (int i = 2; i >= 0; i--) 
	{
		bool* valid;
		UInt8 t = ReadByte(ITE_TEMPERATURE_BASE_REG + i, valid);
		
		// Second chance
		if (!valid || t == 0 || t > 128 )
		{
			IOSleep(500);
			t = ReadByte(ITE_TEMPERATURE_BASE_REG + i, valid);
		}
		
		if (valid && t > 0 && t < 128)
		{
			switch (count) 
			{
				case 0:
				{
					// Heatsink
					TemperatureIndex[0] = i;
					FakeSMCAddKeyCallback("Th0H", "sp78", 2, value, &::Update);
				} break;
				case 1:
				{
					// Northbridge
					TemperatureIndex[1] = i;
					FakeSMCAddKeyCallback("TN0P", "sp78", 2, value, &::Update);
				} break;
			}
			
			count++;
		}			
	}

	// CPU Vcore
	FakeSMCAddKeyCallback("VC0C", "fp2e", 2, value, &::Update);
	FakeSMCAddKeyCallback("VC0c", "ui16", 2, value, &::Update);
	
	// FANs
	FanOffset = GetFNum();
	
	for (int i = 0; i < 5; i++) 
	{
		char key[5];
		bool fanName = FanName[i] && strlen(FanName[i]) > 0;
		
		if ( fanName || ReadRPM(i) > 0)
		{	
			if(fanName)
			{
				snprintf(key, 5, "F%dID", FanOffset + FanCount);
				FakeSMCAddKey(key, "ch8*", strlen(FanName[i]), (char*)FanName[i]);
			}
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			FakeSMCAddKeyCallback(key, "fpe2", 2, value, &::Update);
			
			FanIndex[FanCount++] = i;
		}
	}
	
	UpdateFNum(FanCount);
}

void IT87x::Finish()
{
	FakeSMCRemoveKeyCallback("Th0H");
	FakeSMCRemoveKeyCallback("TN0P");
	FakeSMCRemoveKeyCallback("VC0C");
	FakeSMCRemoveKeyCallback("VC0c");
	for (int i = FanOffset; i < FanOffset + FanCount; i++) 
	{
		char key[5];
		snprintf(key, 5, "F%dAc", i);
		FakeSMCRemoveKeyCallback(key);
	}
	UpdateFNum(-FanCount);
}

void IT87x::Update(const char *key, char *data)
{
	bool* valid;
	
	// Heatsink
	if(CompareKeys(key, "Th0H"))
	{
		char value = ReadByte(ITE_TEMPERATURE_BASE_REG + TemperatureIndex[0], valid);
		
		if(valid)
		{
			data[0] = value;
			data[1] = 0;
		}
	}
	
	// Northbridge
	if(CompareKeys(key, "TN0P"))
	{
		char value = ReadByte(ITE_TEMPERATURE_BASE_REG + TemperatureIndex[1], valid);
		
		if(valid)
		{
			data[0] = value;
			data[1] = 0;
		}
	}
	
	// CPU Vcore
	if(CompareKeys(key, "VC0C"))
	{
		LastVcore = ReadByte(ITE_VOLTAGE_BASE_REG + 0, valid) << 4;
		
		if (valid)
		{
			UInt16 dec = LastVcore / 1000;
			UInt16 frc = LastVcore - (dec * 1000);
			
			UInt16 value = (dec << 14) | (frc << 4);
			
			data[0] = (value & 0xff00) >> 8;
			data[1] = (value & 0x00ff) | 0xb;
		}
	}
	
	if(CompareKeys(key, "VC0c"))
	{
		data[0] = (LastVcore & 0xff00) >> 8;
		data[1] = LastVcore & 0x00ff;
	}
	
	// FANs
	for (int i = FanOffset; i < FanOffset + FanCount; i++)
	{
		char name[5];
		
		snprintf(name, 5, "F%dAc", i);
		
		if(CompareKeys(key, name))
		{
			short value = ReadRPM(FanIndex[key[1] - 48 - FanOffset]);
			
			data[0] = (value >> 6) & 0xff;
			data[1] = (value << 2) & 0xff;
		}
	}
}