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

void ITE::WriteByte(UInt8 reg, UInt8 value)
{
	outb(Address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	outb(Address + ITE_DATA_REGISTER_OFFSET, value);
}

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
	
	// SmartGuardian Setup
	OSDictionary* dictionary = OSDynamicCast(OSDictionary, m_Service->getProperty("SmartGuardian"));
	
	if (dictionary)
	{
		OSBoolean* enabled = OSDynamicCast(OSBoolean, dictionary->getObject("Enabled"));
		
		if (enabled && enabled->getValue())
		{
			for (int i = 0; i < 3; i++) 
			{
				char key[5];
				
				snprintf(key, 5, "Fan%d", i);
				
				OSDictionary* fanInfo = OSDynamicCast(OSDictionary, dictionary->getObject(key));
				
				if (fanInfo)
				{
					const OSBoolean* tmpBool;
					const OSNumber* tmpNumber;
					
					UInt8 control = 0;
					
					if ((tmpBool = OSDynamicCast(OSBoolean, fanInfo->getObject("Temperature Smothing"))) != NULL)
						if (tmpBool->getValue())
						{
							control |= 0x80;
						}
						
					if ((tmpBool = OSDynamicCast(OSBoolean, fanInfo->getObject("Full Speed OFF Temperature Limit"))) != NULL)
						if (tmpBool->getValue())
						{
							control |= 0x40;
						}
					
					if ((tmpBool = OSDynamicCast(OSBoolean, fanInfo->getObject("Spin-Up"))) != NULL)
						if (tmpBool->getValue())
						{
							control |= 0x20;
						}
					
					if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Spin-Up Time"))) != NULL)
						switch (tmpNumber->unsigned16BitValue())
						{
							case 0:
								break;
							case 125:
								control |= 0x08;
								break;
							case 325:
								control |= 0x10;
								break;
							case 1000:
								control |= 0x18;
								break;
						}
					
					if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Slope PWM"))) != NULL)
						switch (tmpNumber->unsigned8BitValue())
						{
							case 1:
								control |= 1;
								break;
							case 2:
								control |= 2;
								break;
							case 4:
								control |= 3;
								break;
							case 8:
								control |= 4;
								break;
							case 16:
								control |= 5;
								break;
							case 32:
								control |= 6;
								break;
							case 64:
								control |= 7;
								break;
						}
					
					// Write control
					WriteByte(ITE_SMARTGUARDIAN_CONTROL[i], control);
					IOSleep(50);
					
					if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Fan Start Temperature"))) != NULL)
					{
						WriteByte(ITE_SMARTGUARDIAN_TEMPERATURE_START[i], tmpNumber->unsigned8BitValue());
						IOSleep(50);
					}
					
					if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Fan Stop Temperature"))) != NULL)
					{
						WriteByte(ITE_SMARTGUARDIAN_TEMPERATURE_STOP[i], tmpNumber->unsigned8BitValue());
						IOSleep(50);
					}
					
					/*if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Full Speed ON Temperature"))) != NULL)
					{
						WriteByte(ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[i], tmpNumber->unsigned8BitValue());
						IOSleep(50);
					}*/
					
					if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Full Speed OFF Temperature"))) != NULL)
					{
						WriteByte(ITE_SMARTGUARDIAN_TEMPERATURE_FULL_OFF[i], tmpNumber->unsigned8BitValue());
						IOSleep(50);
					}
					
					if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Start PWM Value"))) != NULL)
					{
						WriteByte(ITE_SMARTGUARDIAN_START_PWM[i], tmpNumber->unsigned8BitValue());
						IOSleep(50);
					}
					
					if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Force PWM Value"))) != NULL)
					{
						WriteByte(ITE_SMARTGUARDIAN_FORCE_PWM[i], tmpNumber->unsigned8BitValue());
						IOSleep(50);
					}
				}
			}
		}
	}
	
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
			
			// Show SmartGuardian info
			bool* valid;
			UInt8 control = ReadByte(ITE_SMARTGUARDIAN_CONTROL[i], valid);
			
			InfoLog("Fan#%d Temperature Smothing %s", i, (control & 0x80) == 0x80 ? "Enabled" : "Disabled");
			InfoLog("Fan#%d Full Speed Off Temperature Limit %s", i, (control & 0x40) == 0x40 ? "On" : "Off");
			InfoLog("Fan#%d Spin-UP %s", i, (control & 0x20) == 0x20 ? "Enabled" : "Disabled");
						
			switch (control & 0x18) 
			{
				case 0:
					InfoLog("Fan#%d Spin-UP Time %d(ms)",i , 0);
					break;
				case 0x8:
					InfoLog("Fan#%d Spin-UP Time %d(ms)",i , 125);
					break;
				case 0x10:
					InfoLog("Fan#%d Spin-UP Time %d(ms)",i , 325);
					break;
				case 0x18:
					InfoLog("Fan#%d Spin-UP Time %d(ms)",i , 1000);
					break;
			}
			
			switch (control & 0x07) 
			{
				case 0:
					InfoLog("Fan#%d Slope PWM %d",i , 0);
					break;
				case 1:
					InfoLog("Fan#%d Slope PWM %d",i , 1);
					break;
				case 2:
					InfoLog("Fan#%d Slope PWM %d",i , 2);
					break;
				case 3:
					InfoLog("Fan#%d Slope PWM %d",i , 4);
					break;
				case 4:
					InfoLog("Fan#%d Slope PWM %d",i , 8);
					break;
				case 5:
					InfoLog("Fan#%d Slope PWM %d",i , 16);
					break;
				case 6:
					InfoLog("Fan#%d Slope PWM %d",i , 32);
					break;
				case 7:
					InfoLog("Fan#%d Slope PWM %d",i , 64);
					break;
			}
			
			InfoLog("Fan#%d Start Temperature %d",i , ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_START[i], valid));
			InfoLog("Fan#%d Stop Temperature %d",i , ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_STOP[i], valid));
			InfoLog("Fan#%d Full Speed ON Temperature %d",i , ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[i], valid));
			InfoLog("Fan#%d Full Speed OFF Temperature %d",i , ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_FULL_OFF[i], valid));
			InfoLog("Fan#%d Start PWM value %d",i ,ReadByte(ITE_SMARTGUARDIAN_START_PWM[i], valid));
			InfoLog("Fan#%d Force PWM value %d",i ,ReadByte(ITE_SMARTGUARDIAN_FORCE_PWM[i], valid));
			
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