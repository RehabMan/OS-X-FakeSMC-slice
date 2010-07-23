#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>
#include "FakeSMCBinding.h"

class RadeonPlugin : public IOService //IOPCIDevice
{
    OSDeclareDefaultStructors(RadeonPlugin)    
private:
	volatile		UInt8* mmio_base;
	UInt16			device_id;
	IOPCIDevice *	VCard;
	IOMemoryMap *	mmio;
	FakeSMCBinding* m_Binding;
protected:
	//	IOWorkLoop *		TZWorkLoop;
	//	IOTimerEventSource * TZPollTimer;

	
public:
	IOReturn Update(const char* key, char* data);
//	virtual int			update(int keyN);
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual bool		init(OSDictionary *properties=0);
	virtual void		free(void);
	virtual void		stop(IOService *provider);
	//	IOReturn			poller( void );
};

class Binding : public FakeSMCBinding 
{
private:
	RadeonPlugin* m_Monitor;
	
public:
	Binding(RadeonPlugin* monitor)
	{
		m_Monitor = monitor;
	}
	
	virtual IOReturn OnKeyRead(const char* key, char* data)
	{
		return m_Monitor->Update(key, data);
		
	};
	
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data)
	{return kIOReturnUnsupported;};
};