#include <IOKit/IOService.h>
#include "IOKit/acpi/IOACPIPlatformDevice.h"
#include <IOKit/IOTimerEventSource.h>

class TZPlugin : public IOService
{
    OSDeclareDefaultStructors(TZPlugin)    
private:
	int TCount, FCount;
	IOACPIPlatformDevice * TZDevice;
protected:
	//	void	Update(SMCData node);
	IOWorkLoop *		TZWorkLoop;
	IOTimerEventSource * TZPollTimer;
		
public:
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual bool		init(OSDictionary *properties=0);
	virtual void		free(void);
	virtual void		stop(IOService *provider);
	IOReturn			poller( void );
};

