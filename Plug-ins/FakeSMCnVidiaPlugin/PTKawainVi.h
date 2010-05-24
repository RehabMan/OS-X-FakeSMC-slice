#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>
#include <fakesmc.h>

void AddKey (const char*, uint8_t, char*);

class PTKawainVi : public IOPCIDevice {
    OSDeclareDefaultStructors(PTKawainVi)    
public:
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start	(IOService *provider);
	virtual bool		init	(OSDictionary *properties=0);
	virtual void		free (void);
	virtual void		stop (IOService *provider);
	virtual IOReturn    LoopTimerEvent(void);
private:
	virtual IOWorkLoop	*getWorkLoop(void) const;
	volatile UInt8* nvio_base;
	UInt16 device_id;
	int arch;
	void get_gpu_arch ();
	float offset;
	float slope;
	IOPCIDevice * NVCard;
	IOMemoryMap * nvio;
	IOPhysicalAddress nvio_base_phys;
	IOWorkLoop *                    WorkLoop;
	IOTimerEventSource *    TimerEventSource;
};
