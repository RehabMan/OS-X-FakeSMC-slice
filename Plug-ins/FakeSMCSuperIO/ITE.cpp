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
			InfoLog("Found unsupported ITE chip ID=0x%x on ADDRESS=0x%x", chipID, Address);
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
			InfoLog("SmartGuardian Mode activated for all FANs");
			
			bool* valid;
			UInt8 control = ReadByte(ITE_SMARTGUARDIAN_MAIN_CONTROL, valid);
			
			control |= 0x7;
			
			WriteByte(ITE_SMARTGUARDIAN_MAIN_CONTROL, control);
		}
		
		for (int i = 0; i < 5; i++) 
		{
			UInt8 control;
			char key[5];
			
			snprintf(key, 5, "Fan%d", i);
			
			OSDictionary* fanInfo = OSDynamicCast(OSDictionary, dictionary->getObject(key));
			
			if (fanInfo)
			{				
				enabled = OSDynamicCast(OSBoolean, fanInfo->getObject("Enabled"));
				
				if (enabled && enabled->getValue())
				{
					InfoLog("SmartGuardian using custom configuration for Fan#%d", i);
					
					const OSBoolean* tmpBool;
					const OSNumber* tmpNumber;
					const OSString* tmpString;
					
					if (i < 3) 
					{
						// SmartGuardian FAN Control
						control = 0;
						
						if ((tmpBool = OSDynamicCast(OSBoolean, fanInfo->getObject("PWM Smoothing"))) != NULL)
						{
							if (tmpBool->getValue())
							{
								control |= 0x80;
							}
						}
							
						if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Slope PWM"))) != NULL)
						{
							switch (tmpNumber->unsigned8BitValue())
							{
								case 1:
								case 2:
								case 4:
								case 8:
								case 16:
								case 32:
								case 64:
									control |= tmpNumber->unsigned8BitValue();
									break;
							}
						}
						
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

						// PWM Control
						control = 0;
						
						if ((tmpBool = OSDynamicCast(OSBoolean, fanInfo->getObject("PWM Automatic Operation"))) != NULL)
						{
							control |= 0x80;
							
							if ((tmpString = OSDynamicCast(OSString, fanInfo->getObject("Automatic Operation Temperature Input"))) != NULL)
							{
								if (tmpString->isEqualTo("TMPPIN1"))
								{
									control |= 0x0;
								}
								else if (tmpString->isEqualTo("TMPPIN2"))
								{
									control |= 0x1;
								}
								else if (tmpString->isEqualTo("TMPPIN3"))
								{
									control |= 0x2;
								}
								else 
								{
									control |= 0x3;
								}
							}
							
							WriteByte(ITE_SMARTGUARDIAN_PWM_CONTROL[i], control);
							IOSleep(50);
						}
						else if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Software Operation PWM Value"))) != NULL)
						{
							control |= (tmpNumber->unsigned8BitValue() & 0x7f);
							
							WriteByte(ITE_SMARTGUARDIAN_PWM_CONTROL[i], control);
							IOSleep(50);
						}
					}
					
					if ((tmpNumber = OSDynamicCast(OSNumber, fanInfo->getObject("Start PWM Value"))) != NULL)
					{
						WriteByte(ITE_SMARTGUARDIAN_START_PWM[i], tmpNumber->unsigned8BitValue());
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
			UInt8 control;
			
			if (i < 3) 
			{
				control = ReadByte(ITE_SMARTGUARDIAN_CONTROL[i], valid);
				
				InfoLog("Fan#%d PWM Smothing %s", i, (control & 0x80) == 0x80 ? "Enabled" : "Disabled");	
				InfoLog("Fan#%d Slope PWM %d", i, control & 0x7f);
				
				InfoLog("Fan#%d Start Temperature %d",i , ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_START[i], valid));
				InfoLog("Fan#%d Stop Temperature %d",i , ReadByte(ITE_SMARTGUARDIAN_TEMPERATURE_STOP[i], valid));
			}

			control = ReadByte(ITE_SMARTGUARDIAN_PWM_CONTROL[i], valid);
			
			InfoLog("Fan#%d PWM Automatic Operation %s", i, (control & 0x80) == 0x80 ? "Enabled" : "Disabled");
			
			if ((control & 0x80) == 0x80)
			{
				switch (control & 0x3) 
				{
					case 0:
						InfoLog("Fan#%d Automatic Operation Temperature Input TMPPIN1", i);
						break;
					case 1:
						InfoLog("Fan#%d Automatic Operation Temperature Input TMPPIN2", i);
						break;
					case 2:
						InfoLog("Fan#%d Automatic Operation Temperature Input TMPPIN3", i);
						break;
					case 3:
						InfoLog("Fan#%d Automatic Operation Temperature Input RESERVED", i);
						break;
				}
			}
			else 
			{
				InfoLog("Fan#%d Software Operation PWM Value %d", i, control & 0x7f);
			}

			
			InfoLog("Fan#%d Start PWM value %d",i ,ReadByte(ITE_SMARTGUARDIAN_START_PWM[i], valid));
			
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