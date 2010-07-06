/*
 *  SuperIO.cpp
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#include "SuperIO.h"

void SuperIO::OnKeyRead(const char* key, char* data)
{
	for (Sensor* sensor = (Sensor*)m_Sensor; sensor; sensor = (Sensor*)sensor->Next)
	{
		if (CompareKeys(sensor->GetKey(), key))
		{
			sensor->OnKeyRead(key, data);
		}
	}
}

void SuperIO::OnKeyWrite(const char* key, char* data)
{
	for (Sensor* sensor = (Sensor*)m_Sensor; sensor; sensor = (Sensor*)sensor->Next)
	{
		if (CompareKeys(sensor->GetKey(), key))
		{
			sensor->OnKeyWrite(key, data);
		}
	}
}

void SuperIO::ControllersTimerEvent()
{
	for (Binding* controller = m_Controller; controller; controller = controller->Next)
	{
		((Controller*)controller)->TimerEvent();
	}
}

void SuperIO::AddSensor(Binding* sensor)
{
	sensor->Next = m_Sensor;
	m_Sensor = sensor;
}

void SuperIO::AddController(Binding* controller)
{	
	controller->Next = m_Controller;
	m_Controller = controller;
}

void SuperIO::FlushList(Binding* start)
{
	Binding* iterator = start;
	
	while (iterator)
	{
		Binding* next = iterator->Next;
		
		delete iterator;
		
		iterator = next;
	}
}

void SuperIO::FlushSensors()
{
	FlushList(m_Sensor);
}

void SuperIO::FlushControllers()
{
	FlushList(m_Controller);
}

UInt8 SuperIO::ListenPortByte(UInt8 reg)
{
	outb(m_RegisterPort, reg);
	return inb(m_ValuePort);
}

UInt16 SuperIO::ListenPortWord(UInt8 reg)
{
	return ((SuperIO::ListenPortByte(reg) << 8) | SuperIO::ListenPortByte(reg + 1));
}

void SuperIO::Select(UInt8 logicalDeviceNumber)
{
	outb(m_RegisterPort, SUPERIO_DEVCIE_SELECT_REGISTER);
	outb(m_ValuePort, logicalDeviceNumber);
}

int	SuperIO::GetNextFanIndex()
{
	char key[5];
	int offset;
	
	int id = GetNextUnusedKey(KEY_FORMAT_FAN_ID, key);
	int ac = GetNextUnusedKey(KEY_FORMAT_FAN_SPEED, key);
	
	if (id != -1 || ac != -1)
	{
		offset = id > ac ? id : ac;
	}
	
	return offset;
}

const char* SuperIO::GetModelName()
{
	switch (m_Model) 
	{
        case F71858: return "Fintek F71858";
        case F71862: return "Fintek F71862";
        case F71869: return "Fintek F71869";
        case F71882: return "Fintek F71882";
        case F71889ED: return "Fintek F71889ED";
        case F71889F: return "Fintek F71889F";
        case IT8512F: return "ITE IT8512F";
        case IT8712F: return "ITE IT8712F";
        case IT8716F: return "ITE IT8716F";
        case IT8718F: return "ITE IT8718F";
        case IT8720F: return "ITE IT8720F";
        case IT8726F: return "ITE IT8726F";
        case IT8752F: return "ITE IT8752F";
        case W83627DHG: return "Winbond W83627DHG";
        case W83627DHGP: return "Winbond W83627DHG-P";
        case W83627EHF: return "Winbond W83627EHF";
        case W83627HF: return "Winbond W83627HF";
        case W83627THF: return "Winbond W83627THF";
        case W83667HG: return "Winbond W83667HG";
        case W83667HGB: return "Winbond W83667HG-B";
        case W83687THF: return "Winbond W83687THF";
			//Slice
        //case W83977CTF: return "Winbond W83977CTF";
        //case W83977EF: return "Winbond W83977EF";
        //case W83977FA: return "Winbond W83977FA";
        //case W83977TF: return "Winbond W83977TF";
        //case W83977ATF: return "Winbond W83977ATF";
        //case W83977AF: return "Winbond W83977AF";
        case W83627SF: return "Winbond W83627SF";
        case W83697HF: return "Winbond W83697HF";
        //case W83L517D: return "Winbond W83L517D";
        case W83637HF: return "Winbond W83637HF";
        case W83627UHG: return "Winbond W83627UHG";
        case W83697SF: return "Winbond W83697SF";
        //case W83877F:  return "Winbond W83877F";
        //case W83877AF: return "Winbond W83877AF";
        //case W83877TF: return "Winbond W83877TF";
        //case W83877ATF: return "Winbond W83877ATF";
			
		case LPC47B27x: return "SMSC LPC47B27x";
		case LPC47B37x: return "SMSC LPC47B37x";
		case LPC47B397_NC: return "SMSC LPC47B397_NC";
		case LPC47M10x_112_13x: return "SMSC LPC47M10x/112/13x";
		case LPC47M14x: return "SMSC LPC47M14x";
		case LPC47M15x_192_997: return "SMSC LPC47M15x/192/997";
		case LPC47M172: return "SMSC LPC47M172";
		case LPC47M182: return "SMSC LPC47M182";
		case LPC47M233: return "SMSC LPC47M233";
		case LPC47M292: return "SMSC LPC47M292";
		case LPC47N252: return "SMSC LPC47N252";
		case LPC47S42x: return "SMSC LPC47S42x";
		case LPC47S45x: return "SMSC LPC47S45x";
		case LPC47U33x: return "SMSC LPC47U33x";
		case SCH3112: return "SMSC SCH3112";
		case SCH3114: return "SMSC SCH3114";
		case SCH3116: return "SMSC SCH3116";
		case SCH4307: return "SMSC SCH4307";
		case SCH5127: return "SMSC SCH5127";
		case SCH5307_NS: return "SMSC SCH5307-NS";
		case SCH5317_1:
		case SCH5317_2: return "SMSC SCH5317";
    };
	
	return "Unknown";
}

void SuperIO::LoadConfiguration(IOService* provider)
{
	m_Service = provider;
	
	OSArray* fanIDs = OSDynamicCast(OSArray, provider->getProperty("Fan Names"));
	
	if (fanIDs) 
		fanIDs = OSArray::withArray(fanIDs);
	
    if (fanIDs) 
	{
		UInt32 count = fanIDs->getCount();
		
		if(count > 5) count = 5;
		
		for (UInt32 i = 0; i < count; i++)
		{
			OSString* name = OSDynamicCast(OSString, fanIDs->getObject(i)); 
			m_FanName[i] = name->getCStringNoCopy();
		}
		
		fanIDs->release();
    }
	
	// Fan Control
	OSDictionary* fanControlConfig = OSDynamicCast(OSDictionary, m_Service->getProperty("Fan Control"));
	
	if (fanControlConfig)
	{
		OSBoolean* enabled = OSDynamicCast(OSBoolean, fanControlConfig->getObject("Enabled"));
		
		if (enabled && enabled->getValue())
		{
			m_FanControl = true;
			
			InfoLog("Software FAN control enabled");
			
			enabled = OSDynamicCast(OSBoolean, fanControlConfig->getObject("Voltage Control"));
			
			if (enabled && enabled->getValue())
			{
				InfoLog("Fan Control in voltage mode");
				
				m_FanVoltageControlled = true;
			}
			else
			{
				InfoLog("Fan Control in PWM mode");
			}
		}
	}
	
	cpuid_update_generic_info();
}

bool SuperIO::Probe()
{
	m_Model = UnknownModel;
	
	// To be safe
	Exit();
	
	for (UInt8 i = 0; i < GetPortsCount(); i++)
	{
		SelectPort(i);
		
		Enter();
		
		if (ProbePort())
		{
			Exit();
			
			return true;
		}
		
		Exit();
	}
	
	return false;
}

void SuperIO::Start()
{
}

void SuperIO::Stop()
{
	FlushSensors();
	FlushControllers();
	UpdateFNum();
}