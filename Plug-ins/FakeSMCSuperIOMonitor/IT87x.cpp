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

void IT87xTemperatureSensor::OnKeyRead(__unused const char* key, char* data)
{
	bool* valid;
	
	UInt8 value = IT87x_ReadTemperature(m_Address, m_Offset, valid);
	
	if(valid)
	{
		data[0] = value;
		data[1] = 0;
	}
}

void IT87xTemperatureSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void IT87xVoltageSensor::OnKeyRead(__unused const char* key, char* data)
{
	bool* valid;
	
	UInt16 V = IT87x_ReadByte(m_Address, ITE_VOLTAGE_BASE_REG + m_Offset, valid) << 4;
	
	if (valid)
	{
		UInt16 value = fp2e_Encode(V);
		
		data[0] = (value & 0xff00) >> 8;
		data[1] = value & 0x00ff;
	}
}

void IT87xVoltageSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void IT87xTachometerSensor::FanForcePWM(UInt16 slope)
{
	InfoLog("Forcing Fan #%d to %d%%", m_Offset, slope);
	
	outb(m_Address+ITE_ADDRESS_REGISTER_OFFSET, ITE_FAN_FORCE_PWM_REG[m_Offset]);
	outb(m_Address+ITE_DATA_REGISTER_OFFSET, slope);
}

void IT87xTachometerSensor::OnKeyRead(__unused const char* key, char* data)
{
	bool* valid;
	
	UInt16 value = IT87x_ReadTachometer(m_Address, m_Offset, valid);
	
	if(valid)
	{
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	}
}

void IT87xTachometerSensor::OnKeyWrite(const char* key, char* data)
{
	if (m_FanControl && CompareKeys(m_FminKey, key))
	{
		UInt16 rpm = (UInt16(data[0]) << 8 | (data[1] & 0xff)) >> 2;
		
		if (m_MaxRpm != m_MinRpm)
		{
			UInt8 slope = UInt8(100*(rpm - m_MinRpm) / (m_MaxRpm - m_MinRpm));
			
			InfoLog("Fan #%d RPM=%d%%, MAX%d, MIN%d",m_Offset, rpm, slope, m_MaxRpm, m_MinRpm);
			
			outb(m_Address + ITE_ADDRESS_REGISTER_OFFSET, ITE_START_PWM_VALUE_REG[m_Offset]);
			outb(m_Address + ITE_DATA_REGISTER_OFFSET, slope);
		}
	}
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

bool IT87x::Probe()
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
		
		vendorId = IT87x_ReadByte(Address, ITE_VENDOR_ID_REGISTER, valid);
		
		if (!valid || vendorId != ITE_VENDOR_ID)
			continue;
		
		if ((IT87x_ReadByte(Address, ITE_CONFIGURATION_REGISTER, valid) & 0x10) == 0)
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
	bool* valid;
	
	// Temperature semi-autodetection
	
	int count = 0;
	
	for (int i = 2; i >= 0; i--) 
	{
		UInt8 t = IT87x_ReadTemperature(Address, i, valid);
		
		// Second chance
		if (!valid || t == 0 || t > 128 )
		{
			IOSleep(500);
			t = IT87x_ReadTemperature(Address, i, valid);
		}
		
		if (valid && t > 0 && t < 128)
		{
			switch (count) 
			{
				case 0:
				{
					// Heatsink
					RegisterSensor(new IT87xTemperatureSensor(Address, i, "Th0H", "sp78", 2));
				} break;
				case 1:
				{
					// Northbridge
					RegisterSensor(new IT87xTemperatureSensor(Address, i, "TN0P", "sp78", 2));
				} break;
			}
			
			count++;
		}			
	}

	// CPU Vcore
	RegisterSensor(new IT87xVoltageSensor(Address, 0, "VC0C", "fp2e", 2));
	//FakeSMCAddKey("VC0c", "ui16", 2, value, this);
	
	// FANs
	FanOffset = GetFNum();
	
	for (int i = 0; i < 5; i++) 
	{
		char key[5];
		bool fanName = FanName[i] && strlen(FanName[i]) > 0;
		
		if ( fanName || IT87x_ReadTachometer(Address, i, valid) > 0)
		{	
			if(valid || fanName)
			{
				snprintf(key, 5, "F%dID", FanOffset + FanCount);
				FakeSMCAddKey(key, "ch8*", strlen(FanName[i]), (char*)FanName[i]);
			}
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			RegisterSensor(new IT87xTachometerSensor(Address, i, FanOffset + FanCount, m_FanControl, key, "fpe2", 2));
			
			FanIndex[FanCount++] = i;
		}
	}
	
	UpdateFNum(FanCount);
}

void IT87x::Finish()
{
	FlushSensors();
	UpdateFNum(-FanCount);
}