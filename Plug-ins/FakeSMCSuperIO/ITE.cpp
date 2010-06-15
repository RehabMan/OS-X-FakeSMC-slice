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
#include "ITESensors.h"
#include "SmartGuardianController.h"

UInt8 ITE::ReadByte(UInt8 index, bool* valid)
{
	outb(Address + ITE_ADDRESS_REGISTER_OFFSET, index);
	
	UInt8 value = inb(Address + ITE_DATA_REGISTER_OFFSET);
	
	valid = (bool*)(index == inb(Address + ITE_DATA_REGISTER_OFFSET));
	
	return value;
}

UInt16 ITE::ReadWord(UInt8 index1, UInt8 index2, bool* valid)
{	
	return ITE::ReadByte(index1, valid) << 8 | ITE::ReadByte(index2, valid);
}

SInt16 ITE::ReadTemperature(UInt8 index)
{
	bool* valid;
	return ITE::ReadByte(ITE_TEMPERATURE_BASE_REG + index, valid);
}

SInt16 ITE::ReadVoltage(UInt8 index)
{
	bool* valid;
	return ITE::ReadByte(ITE_VOLTAGE_BASE_REG + index, valid) << 4;
}

SInt16 ITE::ReadTachometer(UInt8 index)
{
	bool* valid;
	int value = ITE::ReadByte(ITE_FAN_TACHOMETER_REG[index], valid);
	
	value |= ITE::ReadByte(ITE_FAN_TACHOMETER_EXT_REG[index], valid) << 8;
	
	return value > 0x3f && value < 0xffff ? (float)(1350000 + value) / (float)(value * 2) : 0;
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
	DebugLog("Probing ITE...");
	
	Model = UnknownModel;
	
	for (int i = 0; i < ITE_PORTS_COUNT; i++) 
	{
		RegisterPort	= ITE_PORT[i];
		ValuePort		= ITE_PORT[i] + 1;
		
		Enter();
		
		UInt16 chipID = ListenPortWord(SUPERIO_CHIP_ID_REGISTER);
		
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
		
		Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
		IOSleep(1000);
		
		UInt16 verify = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
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

void ITE::Init()
{
	// Temperature semi-autodetection
	
	int count = 0;
	
	for (int i = 2; i >= 0; i--) 
	{		
		UInt8 t = ReadTemperature(i);
		
		// Second chance
		if (t == 0 || t > 128 )
		{
			IOSleep(1000);
			t = ReadTemperature(i);
		}
		
		if (t > 0 && t < 128)
		{
			switch (count) 
			{
				case 0:
				{
					// Heatsink
					Bind(new ITETemperatureSensor(this, i, "Th0H", "sp78", 2));
				} break;
				case 1:
				{
					// Northbridge
					Bind(new ITETemperatureSensor(this, i, "TN0P", "sp78", 2));
				} break;
			}
			
			count++;
		}			
	}

	// CPU Vcore
	Bind(new ITEVoltageSensor(this, 0, "VC0C", "fp2e", 2));
	
	// FANs
	/*OSDictionary * dictionary = OSDictionary::withCapacity(0);

	if (dictionary)
	{
		OSDictionary * faninfo = OSDictionary::withCapacity(0);
		
		UInt8 control = ReadByte(ITE_SMARTGUARDIAN_CONTROL[m_Offset]);
		
		setBoolean("Temperature Smothing", (control & 0x80) == 0x80, faninfo);
		setBoolean("Temperature Limit of Full speed OFF", (control & 0x40) == 0x40, faninfo);
		setBoolean("Fan spin-UP", (control & 0x20) == 0x20, faninfo);
		
		switch (control & 0x18) 
		{
			case 0:
				setNumber("Fan spin-UP Time (ms)", 0, faninfo);
				break;
			case 0x8:
				setNumber("Fan spin-UP Time (ms)", 125, faninfo);
				break;
			case 0x10:
				setNumber("Fan spin-UP Time (ms)", 325, faninfo);
				break;
			case 0x18:
				setNumber("Fan spin-UP Time (ms)", 1000, faninfo);
				break;
		}
		
		switch (control & 0x07) 
		{
			case 0:
				setNumber("Slope PWM", 0, faninfo);
				break;
			case 1:
				setNumber("Slope PWM", 1, faninfo);
				break;
			case 2:
				setNumber("Slope PWM", 2, faninfo);
				break;
			case 3:
				setNumber("Slope PWM", 4, faninfo);
				break;
			case 4:
				setNumber("Slope PWM", 8, faninfo);
				break;
			case 5:
				setNumber("Slope PWM", 16, faninfo);
				break;
			case 6:
				setNumber("Slope PWM", 32, faninfo);
				break;
			case 7:
				setNumber("Slope PWM", 64, faninfo);
				break;
		}
		
		setNumber("Temperature limit of Fan stop", m_Provider->ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_OFF[m_Offset]), faninfo);
		setNumber("Temperature limit of Fan start", m_Provider->ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_START[m_Offset]), faninfo);
		setNumber("Temperature limit of Fan full speed ON", m_Provider->ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[m_Offset]), faninfo);
		setNumber("Temperature limit of Fan full speed OFF", m_Provider->ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_FULL_OFF[m_Offset]), faninfo);
		setNumber("Start PWM value", m_Provider->ReadByte(ITE_SMARTGUARDIAN_START_PWM[m_Offset]), faninfo);
		
		snprintf(tmpKey, 5, "Fan%d", m_Offset);
		dictionary->setObject(tmpKey, faninfo);
		faninfo->release();
		
		if (newDict)
		{
			m_Provider->GetService()->setProperty("SmartGuardian", dictionary);
			dictionary->release();
		}
	}	*/
	
	FanOffset = GetFNum();
	
	for (int i = 0; i < 5; i++) 
	{
		char key[5];
		bool fanName = FanName[i] && strlen(FanName[i]) > 0;
		
		if (fanName || ReadTachometer(i) > 0)
		{
			if (fanName)
			{
				snprintf(key, 5, "F%dID", FanOffset + FanCount);
				FakeSMCAddKey(key, "ch8*", strlen(FanName[i]), (char*)FanName[i]);
			}
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			Bind(new ITETachometerSensor(this, i, key, "fpe2", 2));
			
			if (m_FanControl)
				Bind(new SmartGuardianController(this, i, FanOffset + FanCount));
			
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