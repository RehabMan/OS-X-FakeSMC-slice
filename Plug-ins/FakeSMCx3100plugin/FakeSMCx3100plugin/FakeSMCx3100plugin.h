#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>
#include "fakesmc.h"

class x3100plugin : public IOService //IOPCIDevice
{
    OSDeclareDefaultStructors(x3100plugin)    
private:
	volatile		UInt8* mmio_base;
	UInt16			device_id;
	IOPCIDevice *	VCard;
	IOMemoryMap *	mmio;
protected:
	//	IOWorkLoop *		TZWorkLoop;
	//	IOTimerEventSource * TZPollTimer;

	
public:
	virtual int			update(int keyN);
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual bool		init(OSDictionary *properties=0);
	virtual void		free(void);
	virtual void		stop(IOService *provider);
	//	IOReturn			poller( void );
};
