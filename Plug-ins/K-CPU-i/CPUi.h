#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <i386/cpuid.h>
#include <i386/proc_reg.h>
#include "FakeSMCBinding.h"

#include <IOCPU.h>


#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define CTL(fid, vid)	(((fid) << 8) | (vid))
#define FID(ctl)		(((ctl) & 0xff00) >> 8)
#define VID(ctl)		((ctl) & 0x00ff)

#define MSR_P4_EBC_FREQUENCY_ID		0x002C
#define MSR_M_PSB_CLOCK_STS			0x00CD
#define MSR_IA32_MPERF				0x00E7
#define MSR_IA32_APERF				0x00E8
#define MSR_IA32_EXT_CONFIG			0x00EE
#define MSR_IA32_CLOCK_MODULATION	0x019A
#define MSR_IA32_THERM_INTERRUPT	0x019B
#define MSR_IA32_THERM_STATUS		0x019C
#define MSR_IA32_TEMPERATURE_TARGET	0x01A2

#define MSR_AMD_CLKCTL				0xc001001b
#define MSR_AMD_FIDVID_CTL			0xc0010041
#define MSR_AMD_FIDVID_STS			0xc0010042
#define MSR_AMD_PSTATE_LIMIT		0xc0010061
#define MSR_AMD_PSTATE_CTL			0xc0010062
#define MSR_AMD_PSTATE_STS			0xc0010063
#define MSR_AMD_PSTATE_BASE			0xc0010064
#define MSR_AMD_COFVID_CTL			0xc0010070
#define MSR_AMD_COFVID_STS			0xc0010071

#define keyActive			"Active"
#define keyThrottle			"Throttle"

#define keyInfo				"Info"
#define keyFsbClock			"FsbClock"
#define keyBusClock			"BusClock"
#define keyCpuClock			"CpuClock"
#define keyCpuVendor		"CpuVendor"
#define keyCpuBrand			"CpuBrand"
#define keyCpuModel			"CpuModel"
#define keyCpuFeatures		"CpuFeatures"
#define keyCpuExtFeatures	"CpuExtFeatures"
#define keyCpuCasheSize		"CpuCasheSize"
#define keyCpuCacheL2		"CpuCasheL2"
#define keyCpuCount			"CpuCount"
#define keyCpuCoreTech		"CpuCoreTech"
#define keyCpuMobile		"CpuMobile"
#define keyCpuTjmax			"CpuTjmax"

#define keyPStates			"PStates"
#define keyVID				"Control"
#define keyCID				"CID"

#define keyStatus			"Status"
#define keyPerCpuStatus		"PerCpuStatus"
#define keyName				"Name"
#define keyFrequency		"Frequency"
#define keyVoltage			"Voltage"
#define keyControl			"Control"
#define keyThermal			"Thermal"
#define keySpeedStep		"SpeedStep"

extern "C" void mp_rendezvous_no_intrs(void (*action_func)(void *), void * arg);
extern "C" int cpu_number(void);

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

struct PState 
{
	union 
	{
		UInt16 Control;
		struct 
		{
			UInt8 VID;	// Voltage ID
			UInt8 FID;	// Frequency ID
		};
	};
	
	UInt8	DID;		// DID
	UInt8	CID;		// Compare ID
};

#define MaxCpuCount		16
#define MaxPStateCount	32


UInt32					Frequency[MaxCpuCount];
UInt8					CpuTjmax;

//IOSimpleLock *			SimpleLock;

class CPUi : public IOService {
	OSDeclareDefaultStructors(CPUi)	
private:
	bool					Active;
	
	bool					LoopLock;
	
	UInt32					BusClock;
	UInt32					FSBClock;
	
	bool					CpuMobile;
	bool					CpuNonIntegerBusRatio;
	bool					CpuDynamicFSB;
	UInt8					CpuCoreTech;
	UInt32					CpuSignature;
	UInt32					CpuClock;
	
	UInt8					CpuCount;
	IOCPU *					CpuArray[MaxCpuCount];
	FrequencySensor*		FreqBinding[MaxCpuCount];
	TemperatureSensor*		TempBinding[MaxCpuCount];
	
	PState					PStateInitial;
	PState					PStateMinimum;
	PState					PStateMaximum;
	
	UInt8					PStatesCount;
	PState					PStates[MaxPStateCount];
	
	
	IOWorkLoop *			WorkLoop;
	IOTimerEventSource *	TimerEventSource;
	
	//UInt32					Frequency[MaxCpuCount];
	UInt32					Voltage[MaxCpuCount];
	float					Performance[MaxCpuCount];
	
	void	Activate(void);
	void	Deactivate(void);
public:
	virtual IOReturn	loopTimerEvent(void);
	
	virtual IOReturn	setProperties(OSObject * properties);
	virtual IOService * probe(IOService * provider, SInt32 * score);
	virtual bool		start(IOService * provider);
	virtual void		stop(IOService * provider);
	virtual void		free(void);
};

inline UInt32 IntelGetFrequency(UInt8 fid, UInt32 fsb) {
	UInt32 multiplier = fid & 0x1f;					// = 0x08
	bool half = fid & 0x40;							// = 0x01
	bool dfsb = fid & 0x80;							// = 0x00
	UInt32 halffsb = (fsb + 1) >> 1;				// = 200
	UInt32 frequency = (multiplier * fsb);			// = 3200
	return (frequency + (half * halffsb)) >> dfsb;	// = 3200 + 200 = 3400
}

inline UInt32 IntelGetVoltage(UInt8 vid) {
	return 700 + ((vid & 0x3F) << 4);
}

UInt8	GlobalThermal[MaxCpuCount];
bool	GlobalThermalValueIsObsolete[MaxCpuCount];
PState 	GlobalState[MaxCpuCount];

inline void IntelWaitForSts(void) {
	UInt32 inline_timeout = 100000;
	while (rdmsr64(MSR_IA32_PERF_STS) & (1 << 21)) { if (!inline_timeout--) break; }
}


inline void IntelState(void * magic)
{
	UInt32 i = cpu_number();
	GlobalState[i].Control = rdmsr64(MSR_IA32_PERF_STS) & 0xFFFF;
}

inline void IntelThermal(void * magic)
{
	UInt32 i = cpu_number();
	UInt64 msr = rdmsr64(MSR_IA32_THERM_STATUS);
	if (msr & 0x80000000) {
		GlobalThermal[i] = (msr >> 16) & 0x7F;
		GlobalThermalValueIsObsolete[i]=false;
	}
}

// GetPlistValue
UInt32 getPlistValue(OSDictionary * dictionary, const char * symbol)
{
	OSObject * object = 0;
	OSBoolean * boolean = false;
	OSNumber * number = 0;
	OSString * string = 0;
	object = dictionary->getObject(symbol);
	if (object && (OSTypeIDInst(object) == OSTypeID(OSBoolean)))
	{
		boolean = OSDynamicCast(OSBoolean, object);
		return boolean->getValue();
	}
	if (object && (OSTypeIDInst(object) == OSTypeID(OSNumber)))
	{
		number = OSDynamicCast(OSNumber, object);
		return number->unsigned32BitValue();
	}
	if (object && (OSTypeIDInst(object) == OSTypeID(OSString)))
	{
		string = OSDynamicCast(OSString, object);
		// Implement string to number conversion
	}
	return 0;
}

// GetPlistValue
UInt32 getPlistValue(OSDictionary * dictionary, const char * symbol, const char * subdictionary)
{
	OSObject * object = dictionary->getObject(subdictionary);
	if (object && (OSTypeIDInst(object) == OSTypeID(OSDictionary)))
	{
		OSDictionary * newdictionary = OSDynamicCast(OSDictionary, object);
		return getPlistValue(newdictionary, symbol);
	}
	return 0;
}
