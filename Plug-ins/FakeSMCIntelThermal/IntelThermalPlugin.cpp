/*
 *  IntelThermal.cpp
 *  FakeSMCIntelThermal
 *
 *  Created by Mozodojo on 11/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "IntelThermalPlugin.h"

UInt8 IntelThermalPlugin::GetTemperature(UInt8 index)
{
	if (index > MaxCpuCount)
		return 0;
	
	if (GlobalThermalValueObsolete[index])
	{
		UInt32 magic = 0;
	
		for (int i = 0 ; i < cpuid_info()->core_count; i++)
		{
			mp_rendezvous_no_intrs(IntelThermal, &magic);
		}
	}

	GlobalThermalValueObsolete[index] = true;
	
	if (m_CpuCoreiX)
		return m_TjMaxCoreiX[index] - GlobalThermalValue[index];
	
	return m_TjMax - GlobalThermalValue[index];
}

bool IntelThermalPlugin::Probe()
{
	cpuid_update_generic_info();
	
	if (strcmp(cpuid_info()->cpuid_vendor, CPUID_VID_INTEL) != 0)
	{
		WarningLog("No Intel processor found, kext will not load");
		return false;
	}
	
	if(!(cpuid_info()->cpuid_features & CPUID_FEATURE_MSR))
	{
		WarningLog("Processor does not support Model Specific Registers, kext will not load");
		return false;
	}
	
	m_CpuCount = cpuid_info()->core_count;//cpuid_count_cores();
	
	if(m_CpuCount == 0)
	{
		WarningLog("CPUs not found, kext will not load");
		return false;
	}
	
	UInt32 CpuFamily = cpuid_info()->cpuid_family;
	UInt32 CpuModel = cpuid_info()->cpuid_model;
	UInt32 CpuStepping =  cpuid_info()->cpuid_stepping;
	
	InfoLog("CPU family 0x%x, model 0x%x, stepping 0x%x", (unsigned int)CpuFamily, (unsigned int)CpuModel, (unsigned int)CpuStepping);
	InfoLog("Found %d cores %d threads", m_CpuCount, cpuid_info()->thread_count);
	if (m_TjMax) {
		InfoLog("Manually CPU Tjmax %d", m_TjMax);
		
		return true;		
	}
	switch (CpuFamily)
	{
        case 0x06: 
		{
            switch (CpuModel) 
			{
				case 0x0F: // Intel Core (65nm)
					switch (CpuStepping) 
				{
					case 0x02: // G0
						m_TjMax = 95; break;
					case 0x06: // B2
						switch (m_CpuCount) 
					{
						case 2:
							m_TjMax = 80; break;
						case 4:
							m_TjMax = 90; break;
						default:
							m_TjMax = 85; break;
					}
						m_TjMax = 80; break;
					case 0x0B: // G0
						m_TjMax = 90; break;
					case 0x0D: // M0
						m_TjMax = 85; break;
					default:
						m_TjMax = 85; break;
				}
					break;
					
				case 0x17: // Intel Core (45nm)
					// Mobile CPU ?
					if (rdmsr64(0x17) & (1<<28))
					{
						m_TjMax = 105; break;
					}else
					{
						m_TjMax = 100; break;
					}
					
				case 0x1C: // Intel Atom (45nm)
					switch (CpuStepping)
				{
					case 0x02: // C0
						m_TjMax = 90; break;
					case 0x0A: // A0, B0
						m_TjMax = 100; break;
					default:
						m_TjMax = 90; break;
				} 
					break;
					
				case 0x1A: // Intel Core i7 LGA1366 (45nm)
				case 0x1E: // Intel Core i5, i7 LGA1156 (45nm)
				case 0x25: // Intel Core i3, i5, i7 LGA1156 (32nm)
				case 0x2C: // Intel Core i7 LGA1366 (32nm) 6 Core
					m_CpuCoreiX = true;
					
					for (int i = 0; i < m_CpuCount; i++)
						m_TjMaxCoreiX[i] = (rdmsr64(MSR_IA32_TEMPERATURE_TARGET) >> 16) & 0xFF;
					
					break;
            }
		} 
			break;
	}
	
	if (m_CpuCoreiX) 
	{
		for (int i = 0; i < m_CpuCount; i++)
		{
			InfoLog("CPU#%X Tjmax %d", i, m_TjMaxCoreiX[i]);
		}
	} 
	else 
	{
		InfoLog("CPU Tjmax %d", m_TjMax);
	}

	return true;
}

void IntelThermalPlugin::Start()
{
	for (int i=0; i<m_CpuCount; i++) 
	{
		char value[2];
		
		value[0] = 10;
		value[1] = 0;
		
		char key[5];
		
		snprintf(key, 5, "TC%xD", i);
		
		FakeSMCAddKey(key, "sp78", 0x02, value, this);
	}
}

void IntelThermalPlugin::Stop()
{
	for (int i=0; i<m_CpuCount; i++) 
	{
		char key[5];
		
		snprintf(key, 5, "TC%dD", i);
		
		FakeSMCRemoveKeyBinding(key);
	}
}

IOReturn IntelThermalPlugin::OnKeyRead(const char* key, char* data)
{
	data[0] = GetTemperature(GetIndex(key[2]));
	data[1] = 0;
	
	return kIOReturnSuccess;
}

IOReturn IntelThermalPlugin::OnKeyWrite(__unused const char* key, __unused char* data)
{
	return kIOReturnSuccess;
}