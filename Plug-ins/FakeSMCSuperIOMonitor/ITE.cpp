/*
 *  ITE.cpp
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

#include "ITE.h"
#include "ITETachometerController.h"

void ITETemperatureSensor::OnKeyRead(__unused const char* key, char* data)
{
	bool* valid;
	
	UInt8 value = ITE_ReadTemperature(m_Address, m_Offset, valid);
	
	if(valid)
	{
		data[0] = value;
		data[1] = 0;
	}
}

void ITETemperatureSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void ITEVoltageSensor::OnKeyRead(__unused const char* key, char* data)
{
	bool* valid;
	
	UInt16 V = ITE_ReadByte(m_Address, ITE_VOLTAGE_BASE_REG + m_Offset, valid) << 4;
	
	if (valid)
	{
		UInt16 value = fp2e_Encode(V);
		
		data[0] = (value & 0xff00) >> 8;
		data[1] = value & 0x00ff;
	}
}

void ITEVoltageSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void ITETachometerSensor::OnKeyRead(__unused const char* key, char* data)
{
	bool* valid;
	
	UInt16 value = ITE_ReadTachometer(m_Address, m_Offset, valid);
	
	if(valid)
	{
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	}
}

void ITETachometerSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
}

void ITE::Enter()
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

void ITE::Exit()
{
	outb(RegisterPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(ValuePort, 0x02);
}

bool ITE::Probe()
{
	Model = UnknownModel;
	
	for (int i = 0; i < ITE_PORTS_COUNT; i++) 
	{
		RegisterPort	= ITE_PORT[i];
		ValuePort		= ITE_PORT[i] + 1;
		
		Enter();
		
		UInt16 chipID = ReadWord(SUPERIO_CHIP_ID_REGISTER);
		
		switch (chipID)
		{
			case IT8712F:
			case IT8716F:
			case IT8718F:
			case IT8720F: 
			case IT8726F: 
				Model = (ChipModel)chipID; 
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
		
		vendorId = ITE_ReadByte(Address, ITE_VENDOR_ID_REGISTER, valid);
		
		if (!valid || vendorId != ITE_VENDOR_ID)
			continue;
		
		if ((ITE_ReadByte(Address, ITE_CONFIGURATION_REGISTER, valid) & 0x10) == 0)
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

void ITE::Init()
{	
	bool* valid;
	
	// Temperature semi-autodetection
	
	int count = 0;
	
	for (int i = 2; i >= 0; i--) 
	{		
		UInt8 t = ITE_ReadTemperature(Address, i, valid);
		
		// Second chance
		if (!valid || t == 0 || t > 128 )
		{
			IOSleep(1000);
			t = ITE_ReadTemperature(Address, i, valid);
		}
		
		if (valid && t > 0 && t < 128)
		{
			switch (count) 
			{
				case 0:
				{
					// Heatsink
					Bind(new ITETemperatureSensor(Address, i, "Th0H", "sp78", 2));
				} break;
				case 1:
				{
					// Northbridge
					Bind(new ITETemperatureSensor(Address, i, "TN0P", "sp78", 2));
				} break;
			}
			
			count++;
		}			
	}

	// CPU Vcore
	Bind(new ITEVoltageSensor(Address, 0, "VC0C", "fp2e", 2));
	//FakeSMCAddKey("VC0c", "ui16", 2, value, this);
	
	// FANs
	FanOffset = GetFNum();
	
	for (int i = 0; i < 5; i++) 
	{
		char key[5];
		bool fanName = FanName[i] && strlen(FanName[i]) > 0;
		
		if (fanName || ITE_ReadTachometer(Address, i, valid) > 0)
		{	
			if (!fanName && !valid)
				continue;
			
			if (fanName)
			{
				snprintf(key, 5, "F%dID", FanOffset + FanCount);
				FakeSMCAddKey(key, "ch8*", strlen(FanName[i]), (char*)FanName[i]);
			}
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			Bind(new ITETachometerSensor(Address, i, key, "fpe2", 2));
			
			if (m_FanControl)
				Bind(new ITETachometerController(Address, i, FanOffset + FanCount));
			
			FanIndex[FanCount++] = i;
		}
	}
	
	UpdateFNum(FanCount);
}

void ITE::Finish()
{
	FlushBindings();
	UpdateFNum(-FanCount);
}