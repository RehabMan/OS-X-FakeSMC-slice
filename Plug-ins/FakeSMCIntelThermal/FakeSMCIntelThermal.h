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

#define DebugOn TRUE

#define LogPrefix "FakeSMCIntelThermal: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define MSR_IA32_THERM_STATUS		0x019C
#define MSR_IA32_PERF_STATUS		0x0198;
#define MSR_IA32_TEMPERATURE_TARGET	0x01A2
#define MSR_PLATFORM_INFO			0xCE;

#define MaxCpuCount 128

UInt8	CpuCount = 0;
UInt32	CpuFamily = 0;
UInt32	CpuModel = 0;
UInt32	CpuStepping = 0;

UInt8	TjMax = 100;

bool	CpuCoreiX = false;
UInt	TjMaxCoreiX[MaxCpuCount];

UInt8	Thermal[MaxCpuCount];

extern "C" void mp_rendezvous_no_intrs(void (*action_func)(void *), void * arg);
extern "C" int cpu_number(void);

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
