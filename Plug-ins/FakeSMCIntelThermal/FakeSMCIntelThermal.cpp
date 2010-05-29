/*
 *  FakeSMCIntelThermal.cpp
 *  FakeSMCIntelThermal
 *
 *  Created by mozo on 18/05/10.
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 *	This code includes parts of original Open Hardware Monitor code
 *	Copyright © 2010 Michael Möller. All Rights Reserved.
 */

#include "FakeSMCIntelThermal.h"
#include "cpuid.h"

inline void IntelThermal(void* magic)
{
	UInt32 cpn = cpu_number();
	
	if(cpn < MaxCpuCount)
	{
		if (cpn > CpuCount) CpuCount = cpn + 1;
		
		UInt64 msr = rdmsr64(MSR_IA32_THERM_STATUS);
		
		if (msr & 0x80000000) Thermal[cpn] = (msr >> 16) & 0x7F;
	}
}

static void Update(const char* key, char* data)
{
	UInt32 magic = 0;
	
	mp_rendezvous_no_intrs(IntelThermal, &magic);
	
	UInt8 cpun = GetIndex(key[2]);
	
	if(CpuCoreiX)
	{
		data[0] = TjMaxCoreiX[cpun] - Thermal[cpun];
	}
	else 
	{
		data[0] = TjMax - Thermal[cpun];
	}
	
	data[1] = 0;
}

#define super IOService
OSDefineMetaClassAndStructors(IntelThermalMonitorPlugin, IOService)

IOService* IntelThermalMonitorPlugin::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	
	cpuid_update_generic_info();
	
	if (strcmp(cpuid_info()->cpuid_vendor, CPUID_VID_INTEL) != 0)
	{
		WarningLog("No Intel processor found, kext will not load");
		return NULL;
	}
	
	if(!(cpuid_info()->cpuid_features & CPUID_FEATURE_MSR))
	{
		WarningLog("Processor does not support Model Specific Registers, kext will not load");
		return 0;
	}
	
	CpuCount = /*cpuid_info()->core_count;*/cpuid_count_cores();
	
	if(CpuCount == 0)
	{
		WarningLog("CPUs not found, kext will not load");
		return 0;
	}
	
	CpuFamily = cpuid_info()->cpuid_family;
	CpuModel = cpuid_info()->cpuid_model;
	CpuStepping =  cpuid_info()->cpuid_stepping;
	
	InfoLog("CPU family 0x%x, model 0x%x, stepping 0x%x", CpuFamily, CpuModel, CpuStepping);
	
	switch (CpuFamily)
	{
        case 0x06: 
		{
            switch (CpuModel) 
			{
				case 0x0F: // Intel Core (65nm)
					switch (CpuStepping) 
					{
						case 0x06: // B2
							switch (CpuCount) 
							{
								case 2:
									TjMax = 80 + 10; break;
								case 4:
									TjMax = 90 + 10; break;
								default:
									TjMax = 85 + 10; break;
							}
							TjMax = 80 + 10; break;
						case 0x0B: // G0
							TjMax = 90 + 10; break;
						case 0x0D: // M0
							TjMax = 85 + 10; break;
						default:
							TjMax = 85 + 10; break;
					}
					break;
					
				case 0x17: // Intel Core (45nm)
					// Mobile CPU ?
					if (rdmsr64(0x17) & (1<<28))
					{
						TjMax = 105; break;
					}else
					{
						TjMax = 100; break;
					}
					
				case 0x1C: // Intel Atom (45nm)
					switch (CpuStepping)
					{
						case 0x02: // C0
							TjMax = 90; break;
						case 0x0A: // A0, B0
							TjMax = 100; break;
						default:
							TjMax = 90; break;
					} 
					break;
					
				case 0x1A: // Intel Core i7 LGA1366 (45nm)
				case 0x1E: // Intel Core i5, i7 LGA1156 (45nm)
				case 0x25: // Intel Core i3, i5, i7 LGA1156 (32nm)
				case 0x2C: // Intel Core i7 LGA1366 (32nm) 6 Core
					CpuCoreiX = true;
					
					OSDictionary * iocpu = serviceMatching("IOCPU");
					if (iocpu) 
					{
						OSIterator * iterator = getMatchingServices(iocpu);
						if (iterator) 
						{
							CpuCount = 0;
							
							while (iterator->getNextObject()) 
							{
								CpuCount++;
							}
						}
					}
					
					for (int i = 0; i < CpuCount; i++)
						TjMaxCoreiX[i] = (rdmsr64(MSR_IA32_TEMPERATURE_TARGET) >> 16) & 0xFF;
					
					break;
            }
		} 
			break;
	}
	
	InfoLog("CPU Tjmax %d", TjMax);
	
	return this;
}

bool IntelThermalMonitorPlugin::start(IOService * provider)
{
	if (!super::start(provider)) return false;
	
	for (int i=0; i<CpuCount; i++) 
	{
		char value[2];
		
		value[0] = 10;
		value[1] = 0;
		
		char key[5];
		
		snprintf(key, 5, "TC%xD", i);
		
		FakeSMCAddKeyCallback(key, "sp78", 0x02, value, &Update);
	}
	
	registerService(0);
	
	return true;	
}

bool IntelThermalMonitorPlugin::init(OSDictionary *properties)
{
    super::init(properties);
	
	return true;
}

void IntelThermalMonitorPlugin::stop (IOService* provider)
{
	for (int i=0; i<CpuCount; i++) 
	{
		char key[5];
		
		snprintf(key, 5, "TC%dD", i);
		
		FakeSMCRemoveKeyCallback(key);
	}
	
	super::stop(provider);
}

void IntelThermalMonitorPlugin::free ()
{
	super::free ();
}