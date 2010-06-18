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

void swap (UInt8& a, UInt8& b) {
	UInt8 t=a;
	a=b;
	b=t;
}

void swapInArray (UInt8* array, int no1, int no2) {
	swap(array[no1], array[no2]);
}

bool ITE::Probe()
{
	//DebugLog("Probing ITE...");
	
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

void copyArray (UInt8* dest, UInt8* src, int n){
	for (int i=0; i<n; i++)
		dest[i]=src[i];
}

void ITE::Init()
{
	UInt8 forceArray[5]={0x15, 0x16, 0x17, 0x88, 0x89};
	copyArray(ITE_SMARTGUARDIAN_FORCE_PWM, forceArray, 5);
	 //UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_OFF[5]		= { 0x70, 0x68, 0x60, 0x90, 0x98 };
	 //UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_START[5]	= { 0x71, 0x69, 0x61, 0x91, 0x99 };
	 //UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[5]	= { 0x72, 0x6a, 0x62, 0x92, 0x9a };
	UInt8 startArray[5]={0x63, 0x6b, 0x73, 0x93, 0x9b};
	copyArray(ITE_SMARTGUARDIAN_START_PWM, startArray, 5);
	 //UInt8 ITE_SMARTGUARDIAN_CONTROL[5]				= { 0x74, 0x6c, 0x64, 0x94, 0x9c };
	 //UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_FULL_OFF[5]	= { 0x75, 0x6d, 0x65, 0x95, 0x9d };*/
		
	if (m_AlternateRegisters) {
		swapInArray(ITE_SMARTGUARDIAN_FORCE_PWM, 0, 2);
		swapInArray(ITE_SMARTGUARDIAN_START_PWM, 0, 2);
	}
	
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