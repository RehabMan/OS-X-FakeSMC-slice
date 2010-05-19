/*
 *  FakeSMCIntelThermal.cpp
 *  FakeSMCIntelThermal
 *
 *  Created by mozodojo on 18/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "FakeSMCIntelThermal.h"
#include "cpuid.h"

static void Update(SMCData node)
{
	UInt32 magic = 0;
	
	mp_rendezvous_no_intrs(IntelThermal, &magic);
	
	UInt8 cpun = node->key[2] - 48;
	
	node->data[0] = TjMax - Thermal[cpun];
	node->data[1] = 0;
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
	
	/*if(!(cpuid_info()->cpuid_features & CPUID_FEATURE_EST))
	{
		WarningLog("Processor does not support Enhanced SpeedStep, kext will not load");
		return NULL;
	}*/
	
	CpuCount = cpuid_count_cores();
	
	if(CpuCount == 0)
	{
		WarningLog("CPUs not found, kext will not load");
		return 0;
	}
	
	UInt32 CpuSignature = cpuid_info()->cpuid_signature;
	UInt8 CpuCoreTech = Unknown;
	//bool CpuNonIntegerBusRatio = (rdmsr64(MSR_IA32_PERF_STS) & (1ULL << 46));
	
	// Netburst
	switch (CpuSignature & 0x000FF0) 
	{
		case 0x000F10:
		case 0x000F20:
			CpuCoreTech = IntelNetburstOld;
			break;
		case 0x000F30:
		case 0x000F40:
		case 0x000F60:
			CpuCoreTech = IntelNetburstNew;
			break;
	}
	
	// Core/P-M
	switch (CpuSignature & 0x0006F0) 
	{
		case 0x000690:
		case 0x0006D0:
			CpuCoreTech = IntelPentiumM;
			break;
		case 0x0006E0:
		case 0x0006F0:
			CpuCoreTech = IntelCore;
			break;
	}
	
	// Core/Core45/i7
	switch (CpuSignature & 0x0106F0) 
	{
		case 0x010660:
			CpuCoreTech = IntelCore;
			break;
		case 0x010670:
		case 0x0106D0:
			CpuCoreTech = IntelCore45;
			break;
		case 0x0106C0:
			CpuCoreTech = IntelAtom;
			break;
		case 0x0106A0:
			CpuCoreTech = IntelCoreI7;
			break;
	}
	
	if (CpuCoreTech == Unknown) 
		WarningLog("CPU Core technology unknown");
	
	bool CpuMobile = false;
	bool CpuTjmax15 = false;
	bool CpuDynamicFSB = false;
	
	// Check CPU is mobile
	switch (CpuCoreTech) 
	{
		case IntelPentiumM:
			CpuMobile = true;
			break;
		case IntelNetburstOld:
			CpuMobile = (rdmsr64(MSR_P4_EBC_FREQUENCY_ID) & (1 << 21));
			break;
		case IntelNetburstNew:
			CpuMobile = (rdmsr64(MSR_P4_EBC_FREQUENCY_ID) & (1 << 21));
			break;
		case IntelCoreI7:
		case IntelCore:
		case IntelCore45:
		case IntelAtom:
		{
			CpuMobile = (rdmsr64(MSR_IA32_PLATFORM_ID) & (1 << 28));
			
			if (rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 27)) 
			{
				wrmsr64(MSR_IA32_EXT_CONFIG, (rdmsr64(MSR_IA32_EXT_CONFIG) | (1 << 28))); IOSleep(1);
				CpuDynamicFSB = rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 28);
			}
			
			CpuTjmax15 = (rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 30));
			
			break;
		}
	}
	
	TjMax = 100;
	
	// Find a TjMAX value
	switch (CpuCoreTech) 
	{
		case IntelCore45:
		{
			if (CpuMobile) 
			{
				TjMax += 5;
				
				if (CpuTjmax15)
					TjMax -= 15;
			}
			
			break;
		}
		case IntelAtom:
		{
			if (CpuMobile) 
				TjMax -= 10;
			
			break;
		}
		case IntelCore:
		{
			if ((CpuMobile) && (CpuTjmax15)) 
				TjMax -= 15;
			
			break;
		}
		case IntelCoreI7:
		{
			TjMax = ((rdmsr64(MSR_IA32_TEMPERATURE_TARGET) >> 16) & 0xFF);
			break;
		}
	}	
	
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
		
		snprintf(key, 5, "TC%dD", i);
		
		FakeSMCRegisterKey(key, 0x02, value, &Update);
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
		
		FakeSMCUnregisterKey(key);
	}
	
	super::stop(provider);
}

void IntelThermalMonitorPlugin::free ()
{
	super::free ();
}