#ifndef FakeSMCLPCMonitor_h
#define FakeSMCLPCMonitor_h

#include <IOKit/IOService.h>
#include "IT87x.h"
#include "Winbond.h"

#define DebugOn TRUE

#define LogPrefix "FakeSMCLPCMonitor: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

class LPCMonitorPlugin : public IOService
{
    OSDeclareDefaultStructors(LPCMonitorPlugin)    
private:

protected:
	SuperIO*	superio;
public:	
	virtual bool		init(OSDictionary *properties=0);
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual void		stop(IOService *provider);
	virtual void		free(void);
};

#endif
