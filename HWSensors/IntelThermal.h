/*
 *  HWSensors.h
 *  IntelThermalPlugin
 *
 *  Created by mozo on 30/09/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

#include "cpuid.h"

#define MSR_IA32_THERM_STS			0x019C
#define MSR_IA32_TEMPERATURE_TARGET	0x01A2
//#define MSR_PLATFORM_INFO			0xCE;

#define MaxCpuCount 128

extern "C" void mp_rendezvous_no_intrs(void (*action_func)(void *), void * arg);
extern "C" int cpu_number(void);

static UInt8 GlobalThermalValue[MaxCpuCount];
static UInt16 GlobalControlValue[MaxCpuCount];

enum cpuArch {
	Unknown,
	NetBurst,
	Core,
	Atom,
	Nehalem,
	SandyBridge
};

inline UInt8 get_index(char c)
{
	return c >= 'A' ? c - 55 : c - 48;
};

inline void read_cpu_diode(__unused void* magic)
{
	UInt32 cpn = cpu_number();

	if(cpn < MaxCpuCount) {
		UInt64 msr = rdmsr64(MSR_IA32_THERM_STS);
		if (msr & 0x80000000) { 
			GlobalThermalValue[cpn] = (msr >> 16) & 0x7F;
		}
	}
};

inline void read_cpu_control(__unused void* magic)
{
	UInt32 cpn = cpu_number();
	
	if(cpn < MaxCpuCount) {
		UInt64 msr = rdmsr64(MSR_IA32_PERF_STS);
		GlobalControlValue[cpn] = msr & 0xFFFF;
	}
};

class IntelThermal : public IOService
{
    OSDeclareDefaultStructors(IntelThermal)    
private:
	cpuArch				arch;
	UInt8				count;
	UInt8				tjmax[MaxCpuCount];
	IOService*			fakeSMC;
	
	void				readTjmaxFromMSR();

public:
	virtual bool		init(OSDictionary *properties=0);
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual void		stop(IOService *provider);
	virtual void		free(void);
	
	virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
	
};