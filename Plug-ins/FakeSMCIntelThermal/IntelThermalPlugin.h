/*
 *  IntelThermal.h
 *  FakeSMCIntelThermal
 *
 *  Created by Mozodojo on 11/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include <IOKit/IOLib.h>
#include <libkern/OSTypes.h>
#include <i386/proc_reg.h>

#include "cpuid.h"
#include "FakeSMCBinding.h"

#define DebugOn FALSE

#define LogPrefix "FakeSMCIntelThermal: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define MSR_IA32_THERM_STATUS		0x019C
#define MSR_IA32_PERF_STATUS		0x0198;
#define MSR_IA32_TEMPERATURE_TARGET	0x01A2
#define MSR_PLATFORM_INFO			0xCE;

#define MaxCpuCount 128

extern "C" void mp_rendezvous_no_intrs(void (*action_func)(void *), void * arg);
extern "C" int cpu_number(void);

static UInt8 GlobalThermal[MaxCpuCount];

inline void IntelThermal(__unused void* magic)
{
	UInt32 cpn = cpu_number();
	
	if(cpn < MaxCpuCount)
	{
		UInt64 msr = rdmsr64(MSR_IA32_THERM_STATUS);
		if (msr & 0x80000000) GlobalThermal[cpn] = (msr >> 16) & 0x7F;
	}
};

class IntelThermalPlugin : public FakeSMCBinding
{
private:
	UInt8	m_CpuCount;
	bool	m_CpuCoreiX;
	UInt	m_TjMaxCoreiX[MaxCpuCount];
public:
	UInt8	m_TjMax;
	bool			Probe();
    void			Start();
	void			Stop();
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};