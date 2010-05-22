#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>

#include "fakesmc.h"

#define DebugOn TRUE

#define LogPrefix "FakeSMCLPCMonitor: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

const UInt8 REGISTER_PORT[2] = { 0x2e, 0x4e };
const UInt8 VALUE_PORT[2] = { 0x2f, 0x4f };

// Registers
const UInt8 CONFIGURATION_CONTROL_REGISTER	= 0x02;
const UInt8 DEVCIE_SELECT_REGISTER			= 0x07;
const UInt8 CHIP_ID_REGISTER				= 0x20;
const UInt8 CHIP_REVISION_REGISTER			= 0x21;
const UInt8 BASE_ADDRESS_REGISTER			= 0x60;

// ITE
const UInt8 IT87_ENVIRONMENT_CONTROLLER_LDN = 0x04;

// Winbond, Fintek
const UInt8 FINTEK_VENDOR_ID_REGISTER		= 0x23;
const UInt16 FINTEK_VENDOR_ID				= 0x1934;

const UInt8 WINBOND_HARDWARE_MONITOR_LDN	= 0x0B;

const UInt8 F71858_HARDWARE_MONITOR_LDN		= 0x02;
const UInt8 FINTEK_HARDWARE_MONITOR_LDN		= 0x04;

enum LPCChipType
{
	UnknownType,
    IT87x,
    Winbound,
	Fintek 
};

enum LPCChipModel
{
	UnknownModel = 0,
	
    IT8712F = 0x8712,
    IT8716F = 0x8716,
    IT8718F = 0x8718,
    IT8720F = 0x8720,
    IT8726F = 0x8726,
	
    W83627DHG = 0xA020,
    W83627DHGP = 0xB070,
    W83627EHF = 0x8800,    
    W83627HF = 0x5200,
    W83627THF = 0x8283,
    W83667HG = 0xA510,
    W83667HGB = 0xB350,
    W83687THF = 0x8541,
	
    F71858 = 0x0507,
    F71862 = 0x0601, 
    F71869 = 0x0814,
    F71882 = 0x0541,
    F71889ED = 0x0909,
    F71889F = 0x0723 
};

// ITE
const unsigned char ITE_VENDOR_ID = 0x90;

// ITE Environment Controller
const unsigned char ITE_ADDRESS_REGISTER_OFFSET = 0x05;
const unsigned char ITE_DATA_REGISTER_OFFSET = 0x06;

// ITE Environment Controller Registers    
const unsigned char ITE_CONFIGURATION_REGISTER = 0x00;
const unsigned char ITE_TEMPERATURE_BASE_REG = 0x29;
const unsigned char ITE_VENDOR_ID_REGISTER = 0x58;
const unsigned char ITE_FAN_TACHOMETER_16_BIT_ENABLE_REGISTER = 0x0c;
const unsigned char ITE_FAN_TACHOMETER_REG[] = { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
const unsigned char ITE_FAN_TACHOMETER_EXT_REG[] = { 0x18, 0x19, 0x1a, 0x81, 0x83 };
const unsigned char ITE_VOLTAGE_BASE_REG = 0x20;

// Global

LPCChipType		Type;
LPCChipModel	Model;
const char*		Name;
UInt16			Address;
UInt8			Revision;
UInt8			RegisterPort;
UInt8			ValuePort;

class LPCMonitorPlugin : public IOService
{
    OSDeclareDefaultStructors(LPCMonitorPlugin)    
private:

protected:
	
public:	
	virtual bool		init(OSDictionary *properties=0);
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual void		stop(IOService *provider);
	virtual void		free(void);
};
