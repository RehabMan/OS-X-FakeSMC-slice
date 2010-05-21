#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>

#include "fakesmc.h"

#define DebugOn FALSE

#define LogPrefix "FakeSMCLPCMonitor: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

enum LPCChipModel
{
	Unknown = 0,
	
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

const UInt8 LPCRegisterPort[2] = { 0x2e, 0x4e };
const UInt8 LPCValuePort[2] = { 0x2f, 0x4f };

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

class LPCMonitorPlugin : public IOService
{
    OSDeclareDefaultStructors(LPCMonitorPlugin)    
private:
	const char*		ChipName;
	LPCChipModel	ChipModel;
	UInt8			RegisterPort;
	UInt8			ValuePort;
	
	UInt8	ReadByte(UInt8 reg);
	UInt16	ReadWord(UInt8 reg);
	void	Select(UInt8 logicalDeviceNumber);
	
	void IT87Enter();
	void IT87Exit();
	
	void WinbondFintekEnter();
	void WinbondFintekExit();
	
	void SMSCEnter();
	void SMSCExit();
protected:
public:
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual bool		init(OSDictionary *properties=0);
	virtual void		free(void);
	virtual void		stop(IOService *provider);
};
