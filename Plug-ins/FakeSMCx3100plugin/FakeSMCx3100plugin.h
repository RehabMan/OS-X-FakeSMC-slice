#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>
#include "FakeSMCBinding.h"

class x3100plugin : public IOService //IOPCIDevice
{
    OSDeclareDefaultStructors(x3100plugin)    
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
	void Update(const char* key, char* data);
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
	x3100plugin* m_Monitor;
	
public:
	Binding(x3100plugin* monitor)
	{
		m_Monitor = monitor;
	}
	
	virtual void OnKeyRead(const char* key, char* data)
	{
		m_Monitor->Update(key, data);
	};
	
	virtual void OnKeyWrite(__unused const char* key, __unused char* data) {};
};