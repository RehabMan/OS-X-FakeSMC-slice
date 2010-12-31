/*
 *  HWSensors.h
 *  IntelThermalPlugin
 *
 *  Created by mozo on 30/09/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "IntelThermal.h"
#include "FakeSMC.h"

#define Debug FALSE

#define LogPrefix "IntelThermal: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super IOService
OSDefineMetaClassAndStructors(IntelThermal, IOService)

bool IntelThermal::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
    if (!super::init(properties))
		return false;
	
	return true;
}

IOService* IntelThermal::probe(IOService *provider, SInt32 *score)
{
	DebugLog("Probing...");
	
	if (super::probe(provider, score) != this) return 0;
	
	cpuid_update_generic_info();
	
	if (strcmp(cpuid_info()->cpuid_vendor, CPUID_VID_INTEL) != 0)	{
		WarningLog("No Intel processor found, kext will not load");
		return 0;
	}
	
	if(!(cpuid_info()->cpuid_features & CPUID_FEATURE_MSR))	{
		WarningLog("Processor does not support Model Specific Registers, kext will not load");
		return 0;
	}
	
	count = cpuid_info()->core_count;//cpuid_count_cores();
	
	if(count == 0)	{
		WarningLog("CPUs not found, kext will not load");
		return 0;
	}
	
	UInt32 CpuFamily = cpuid_info()->cpuid_family;
	UInt32 CpuModel = cpuid_info()->cpuid_model;
	UInt32 CpuStepping =  cpuid_info()->cpuid_stepping;
	
	if (OSNumber* number = OSDynamicCast(OSNumber, getProperty("TjMax"))) {
		// User defined Tjmax
		tjmax[0] = number->unsigned32BitValue();
		
		for (int i = 1; i < count; i++)
				tjmax[i] = tjmax[0];
	}
	else { 
		// Calculating Tjmax
		switch (CpuFamily)
		{
			case 0x06: 
			{
				switch (CpuModel) 
				{
					case CPU_MODEL_MEROM: // Intel Core (65nm)
						switch (CpuStepping) 
						{
							case 0x02: // G0
								tjmax[0] = 95; break;
							case 0x06: // B2
								switch (count) 
							{
								case 2:
									tjmax[0] = 80; break;
								case 4:
									tjmax[0] = 90; break;
								default:
									tjmax[0] = 85; break;
							}
								tjmax[0] = 80; break;
							case 0x0B: // G0
								tjmax[0] = 90; break;
							case 0x0D: // M0
								tjmax[0] = 85; break;
							default:
								tjmax[0] = 85; break;
						} break;
						
					case CPU_MODEL_PENRYN: // Intel Core (45nm)
						// Mobile CPU ?
						if (rdmsr64(0x17) & (1<<28)) {
							tjmax[0] = 105; break;
						}
						else {
							tjmax[0] = 100; break;
						}
						
					case CPU_MODEL_ATOM: // Intel Atom (45nm)
						switch (CpuStepping)
						{
							case 0x02: // C0
								tjmax[0] = 90; break;
							case 0x0A: // A0, B0
								tjmax[0] = 100; break;
							default:
								tjmax[0] = 90; break;
						} break;
						
					case CPU_MODEL_NEHALEM:
					case CPU_MODEL_FIELDS:
					case CPU_MODEL_DALES:
					case CPU_MODEL_DALES_32NM:
					case CPU_MODEL_WESTMERE:
					case CPU_MODEL_NEHALEM_EX:
					case CPU_MODEL_WESTMERE_EX:
					{
						nehalemArch = true;
						
						for (int i = 0; i < count; i++) {
							tjmax[i] = (rdmsr64(MSR_IA32_TEMPERATURE_TARGET) >> 16) & 0xFF;
						}
						
					} break;
						
					default:
						WarningLog("Unsupported Intel processor found, kext will not load");
						return 0;
				}
			} break;
				
			default:
				WarningLog("Unknown Intel family processor found, kext will not load");
				return 0;
		}
	}
	
	for (int i = 0; i < count; i++) {
		if (!nehalemArch)
			tjmax[i] = tjmax[0];
		
		key[i] = (char*)IOMalloc(5);
		snprintf(key[i], 5, "TC%XD", i);
	}
		
	return this;
}

bool IntelThermal::start(IOService * provider)
{
	DebugLog("Starting...");
	
	if (!super::start(provider)) return false;
		
    if (!(fakeSMC = waitForService(serviceMatching(kFakeSMCService)))) {
		WarningLog("Can't locate fake SMC device, kext will not load");
		return false;
	}
	
	InfoLog("Based on code by mercurysquad, superhai (C)2008");
	InfoLog("CPU family 0x%x, model 0x%x, stepping 0x%x, cores %d, threads %d, Tjmax %d", 
			cpuid_info()->cpuid_family, 
			cpuid_info()->cpuid_model, 
			cpuid_info()->cpuid_stepping, 
			count, 
			cpuid_info()->thread_count, tjmax[0]);
	
	for (int i = 0; i < count; i++) {
		if (kIOReturnSuccess != fakeSMC->callPlatformFunction(kFakeSMCAddKeyHandler, false, (void *)key[i], (void *)"sp78", (void *)2, this)) {
			WarningLog("Can't add key to fake SMC device, kext will not load");
			return false;
		}
	}
		
	return true;
}

void IntelThermal::stop (IOService* provider)
{
	DebugLog("Stoping...");
	
	super::stop(provider);
}

void IntelThermal::free ()
{
	DebugLog("Freeing...");
	
	super::free ();
}

IOReturn IntelThermal::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	if (functionName->isEqualTo(kFakeSMCGetValueCallback)) {
		const char* name = (const char*)param1;
		void * data = param2;
		//UInt32 size = (UInt64)param3;
		
		if (name && data) {
#if __LP64__
			UInt64 magic;
#else
			UInt32 magic;
#endif			
			UInt32 index = name[2] >= 'A' ? name[2] - 55 : name[2] - 48;
			
			if (index >= 0 && index < count) {
				
				mp_rendezvous_no_intrs(read_cpu_diode, &magic);
				
				UInt16 t = tjmax[index] - GlobalThermalValue[index];
				
				bcopy(&t, data, 2);
				
				return kIOReturnSuccess;
			}			
			
			//DebugLog("cpu index out of bounds");
			
			return kIOReturnBadArgument;
		}
		
		//DebugLog("bad argument key name or data");
		
		return kIOReturnBadArgument;
	}
	
	return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}