/*
 *  Winbond.cpp
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#include "Winbond.h"

void WinbondTemperatureSensor::OnKeyRead(__unused const char* key, char* data)
{
	data[0] = Winbond_ReadTemperature(m_Address, 0);
	data[1] = 0;
}

void WinbondTemperatureSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void WinbondVoltageSensor::OnKeyRead(__unused const char* key, char* data)
{
	float voltage; 
	
	switch (m_Model) 
	{
		case W83627HF:
		case W83627THF:
		case W83687THF:
		{
			UInt8 vrmConfiguration = Winbond_ReadByte(m_Address, m_Offset, 0x18);
			UInt16 V = Winbond_ReadByte(m_Address, m_Offset, WINBOND_VOLTAGE_BASE_REG);
			
			if ((vrmConfiguration & 0x01) == 0)
				voltage = 16.0f * V; // VRM8 formula
			else
				voltage = 4.88f * V + 690.0f; // VRM9 formula
			
			break;
		}
		default:
		{
			UInt16 V = Winbond_ReadByte(m_Address, m_Offset, WINBOND_VOLTAGE_BASE_REG);
			voltage = 8 * V;
			
			break;
		}
	}
	
	UInt16 value = fp2e_Encode(voltage);
	
	data[0] = (value & 0xff00) >> 8;
	data[1] = value & 0x00ff;
}

void WinbondVoltageSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void WinbondTachometerSensor::OnKeyRead(__unused const char* key, char* data)
{
	UInt16 value = Winbond_ReadTachometer(m_Address, m_Offset, false);
	
	data[0] = (value >> 6) & 0xff;
	data[1] = (value << 2) & 0xff;
}

void WinbondTachometerSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void Winbond::Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x87);
}

void Winbond::Exit()
{
	outb(RegisterPort, 0xAA);
}

bool Winbond::Probe()
{	
	Model = UnknownModel;
	
	for (int i = 0; i < WINBOND_PORTS_COUNT; i++) 
	{
		RegisterPort	= WINBOND_PORT[i];
		ValuePort		= WINBOND_PORT[i] + 1;
		
		Enter();
		
		UInt8 id = SuperIO::ReadByte(SUPERIO_CHIP_ID_REGISTER);
		UInt8 revision = SuperIO::ReadByte(SUPERIO_CHIP_REVISION_REGISTER);
		
		switch (id) 
		{		
			case 0x52:
			{
				switch (revision)
				{
					case 0x17:
					case 0x3A:
					case 0x41:
						Model = W83627HF;
						break;
				}
				break;
			}
				
			case 0x82:
			{
				switch (revision)
				{
					case 0x83:
						Model = W83627THF;
						break;
				}
				break;
			}
				
			case 0x85:
			{
				switch (revision)
				{
					case 0x41:
						Model = W83687THF;
						break;
				}
				break;
			}
				
			case 0x88:
			{
				switch (revision & 0xF0)
				{
					case 0x50:
					case 0x60:
						Model = W83627EHF;
						break;
				}
				break;
			}
				
			case 0xA0:
			{
				switch (revision & 0xF0)
				{
					case 0x20: 
						Model = W83627DHG; 
						break;   
				}
				break;
			}
				
			case 0xA5:
			{
				switch (revision & 0xF0)
				{
					case 0x10:
						Model = W83667HG;
						break;
				}
				break;
			}
				
			case 0xB0:
			{
				switch (revision & 0xF0)
				{
					case 0x70:
						Model = W83627DHGP;
						break;
				}
				break;
			}
				
			case 0xB3:
			{
				switch (revision & 0xF0)
				{
					case 0x50:
						Model = W83667HGB;
						break;
				}
				break; 
			}
		}
		
		Select(WINBOND_HARDWARE_MONITOR_LDN);
		
		Address = ReadWord(SUPERIO_BASE_ADDRESS_REGISTER);          
		
		IOSleep(1000);
		
		UInt16 verify = ReadWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
		Exit();
		
		if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
			continue;
		
		if (Model == UnknownModel)
		{
			InfoLog("found unsupported Winbond chip ID=0x%x REVISION=0x%x on ADDRESS=0x%x", id, revision, Address);
			continue;
		} 
		else
		{
			return true;
		}
	}
	
	return false;
}

void Winbond::Init()
{	
	switch (Model) 
	{
		case W83667HG:
		case W83667HGB:
		{
			// do not add temperature sensor registers that read PECI
			UInt8 flag = Winbond_ReadByte(Address, 0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
			
			// Heatsink
			if ((flag & 0x04) == 0)	RegisterSensor(new WinbondTemperatureSensor(Address, 0, "Th0H", "sp78", 2));
		
			/*if ((flag & 0x40) == 0)
			 list.Add(new Sensor(TEMPERATURE_NAME[1], 1, null,
			 SensorType.Temperature, this, parameter));*/
			
			// Northbridge
			RegisterSensor(new WinbondTemperatureSensor(Address, 2, "TN0P", "sp78", 2));
			
			break;
		}
			
		case W83627DHG:        
		case W83627DHGP:
		{
			// do not add temperature sensor registers that read PECI
			UInt8 sel = Winbond_ReadByte(Address, 0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
			
			// Heatsink
			if ((sel & 0x07) == 0) RegisterSensor(new WinbondTemperatureSensor(Address, 0, "Th0H", "sp78", 2));
			
			/*if ((sel & 0x70) == 0)
			 list.Add(new Sensor(TEMPERATURE_NAME[1], 1, null,
			 SensorType.Temperature, this, parameter));*/
			
			// Northbridge
			RegisterSensor(new WinbondTemperatureSensor(Address, 2, "TN0P", "sp78", 2));
			
			break;
		}
			
		default:
		{
			// no PECI support, add all sensors
			// Heatsink
			RegisterSensor(new WinbondTemperatureSensor(Address, 0, "Th0H", "sp78", 2));
			// Northbridge
			RegisterSensor(new WinbondTemperatureSensor(Address, 2, "TN0P", "sp78", 2));
			break;
		}
	}
	
	// CPU Vcore
	RegisterSensor(new WinbondVoltageSensor(Address, Model, 0, "VC0C", "fp2e", 2));
	//FakeSMCAddKey("VC0c", "ui16", 2, value, this);
	
	// FANs
	FanOffset = GetFNum();
	
	Winbond_ReadTachometer(Address, 0, true);
	
	for (int i = 0; i < 5; i++) 
	{
		char key[5];
		bool fanName = FanName[i] && strlen(FanName[i]) > 0;
		
		if (fanName || FanValue[i] > 0)
		{	
			if(fanName)
			{
				snprintf(key, 5, "F%dID", FanOffset + FanCount);
				FakeSMCAddKey(key, "ch8*", strlen(FanName[i]), (char*)FanName[i]);
			}
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			RegisterSensor(new WinbondTachometerSensor(Address, i, key, "fpe2", 2));
			
			FanIndex[FanCount++] = i;
		}
	}
	
	UpdateFNum(FanCount);
}

void Winbond::Finish()
{
	FlushSensors();
	UpdateFNum(-FanCount);
}