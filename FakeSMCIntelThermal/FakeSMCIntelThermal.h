/*
 *  FakeSMCIntelThermal.h
 *  FakeSMCIntelThermal
 *
 *  Created by mozodojo on 18/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <i386/cpuid.h>
#include <i386/proc_reg.h>
#include <fakesmc.h>

#define DebugOn FALSE

#define LogPrefix "FakeSMCIntelThermal: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define MSR_P4_EBC_FREQUENCY_ID		0x002C
#define MSR_M_PSB_CLOCK_STS			0x00CD
#define MSR_IA32_MPERF				0x00E7
#define MSR_IA32_APERF				0x00E8
#define MSR_IA32_EXT_CONFIG			0x00EE
#define MSR_IA32_CLOCK_MODULATION	0x019A
#define MSR_IA32_THERM_INTERRUPT	0x019B
#define MSR_IA32_THERM_STATUS		0x019C
#define MSR_IA32_TEMPERATURE_TARGET	0x01A2

#define MaxCpuCount 128

UInt8 CpuCount = 0;
UInt8 TjMax = 100;
UInt8 Thermal[MaxCpuCount];

enum  {
	Unknown,
	Intel386,
	Intel486,
	IntelPentium,
	IntelPentiumPro,
	IntelPentiumM,
	IntelNetburstOld,
	IntelNetburstNew,
	IntelCore,
	IntelAtom,
	IntelCore45,
	IntelCoreI7,
	AMDK5,
	AMDK6,
	AMDK7,
	AMDK8BC,
	AMDK8D,
	AMDK8E,
	AMDK8NPT,
	AMDK10,
	AMDK11,
	CoreTechCount			
};

extern "C" void mp_rendezvous_no_intrs(void (*action_func)(void *), void * arg);
extern "C" int cpu_number(void);

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

class IntelThermalMonitorPlugin : public IOService
{
    OSDeclareDefaultStructors(IntelThermalMonitorPlugin)    
private:
protected:
public:
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual bool		init(OSDictionary *properties=0);
	virtual void		free(void);
	virtual void		stop(IOService *provider);
};
