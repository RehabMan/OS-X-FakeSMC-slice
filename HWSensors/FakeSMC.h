#ifndef _VIRTUALSMC_H
#define _VIRTUALSMC_H

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "FakeSMCDevice.h"
#include "FakeSMCKey.h"

#define KEY_CPU_HEATSINK_TEMPERATURE		"Th0H"
#define KEY_NORTHBRIDGE_TEMPERATURE			"TN0P"
#define KEY_DIMM_TEMPERATURE				"Tm0P"
#define KEY_AMBIENT_TEMPERATURE				"TA0P"
#define KEY_CPU_VOLTAGE						"VC0C"
#define KEY_CPU_VOLTAGE_RAW					"VC0c"
#define KEY_MEMORY_VOLTAGE					"VM0R"
#define KEY_FAN_NUMBER						"FNum"
#define KEY_CPU_PROXIMITY_TEMPERATURE       "TC0P"

#define	KEY_FORMAT_CPU_DIODE_TEMPERATURE		"TC%XD"
#define	KEY_FORMAT_FAN_ID						"F%XID"
#define	KEY_FORMAT_FAN_SPEED					"F%XAc"
#define	KEY_FORMAT_GPU_DIODE_TEMPERATURE		"TG%XD"
#define	KEY_FORMAT_GPU_BOARD_TEMPERATURE		"TG%XH"
#define KEY_FORMAT_GPU_PROXIMITY_TEMPERATURE	"TG%XP"
#define KEY_FORMAT_NON_APPLE_CPU_FREQUENCY		"FRC%X"
#define KEY_FORMAT_NON_APPLE_CPU_MULTIPLIER		"MC%XC"
#define KEY_FORMAT_NON_APPLE_GPU_FREQUENCY		"FGC%X"

#define TYPE_FPE2							"fpe2"
#define TYPE_FP2E							"fp2e"
#define TYPE_CH8							"ch8*"
#define TYPE_SP78							"sp78"
#define TYPE_UI8							"ui8"
#define TYPE_UI16							"ui16"
#define TYPE_UI32							"ui32"
#define TYPE_SI16                           "si16"
#define TYPE_FLAG                           "flag"
#define TYPE_FREQ							"freq"

#define kFakeSMCService						"FakeSMC"

#define kFakeSMCAddKeyValue					"FakeSMC_AddKeyValue"
#define kFakeSMCAddKeyHandler				"FakeSMC_AddKeyHandler"
#define kFakeSMCSetKeyValue					"FakeSMC_SetKeyValue"
#define kFakeSMCGetKeyValue					"FakeSMC_GetKeyValue"
#define kFakeSMCGetValueCallback			"FakeSMC_GetValueCallback"
#define kFakeSMCSetValueCallback			"FakeSMC_SetValueCallback"

class FakeSMC : public IOService
{
	OSDeclareDefaultStructors(FakeSMC)
	
private:
	FakeSMCDevice		*smcDevice;
	
public:
    virtual bool		init(OSDictionary *dictionary = 0);
    virtual void		free(void);
    virtual IOService	*probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
    virtual void		stop(IOService *provider);
	virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};

#endif