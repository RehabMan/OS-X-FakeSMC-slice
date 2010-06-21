#include <IOKit/IOService.h>
#include "IOKit/acpi/IOACPIPlatformDevice.h"
#include <IOKit/IOTimerEventSource.h>

#include "FakeSMCBinding.h"

class ACPImonitor : public IOService
{
    OSDeclareDefaultStructors(ACPImonitor)    
private:
	int TCount, FCount, SMCx[30];
	UInt8			FanOffset;

	IOACPIPlatformDevice * TZDevice;
	FakeSMCBinding* m_Binding;
	
protected:
	//	void	Update(SMCData node);
//	IOWorkLoop *		TZWorkLoop;
//	IOTimerEventSource * TZPollTimer;
	const char*			FanName[5];
	
public:
	void Update(const char* key, char* data);

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
	ACPImonitor* m_Monitor;
	
public:
	Binding(ACPImonitor* monitor)
	{
		m_Monitor = monitor;
	}
	
	virtual void OnKeyRead(const char* key, char* data)
	{
		m_Monitor->Update(key, data);
	};
	
	virtual void OnKeyWrite(__unused const char* key, __unused char* data) {};
};