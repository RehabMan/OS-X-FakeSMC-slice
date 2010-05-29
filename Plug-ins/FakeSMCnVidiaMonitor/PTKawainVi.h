#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>

class PTKawainVi : public IOService {
    OSDeclareDefaultStructors(PTKawainVi)    
public:
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start	(IOService *provider);
	virtual bool		init	(OSDictionary *properties=0);
	virtual void		free (void);
	virtual void		stop (IOService *provider);
	void UpdateT(const char* key, char* data);
	void UpdateF(const char* key, char* data);
private:
	volatile UInt8* nvio_base;
	UInt16 vendor_id;
	UInt16 device_id;
	int arch;
	void get_gpu_arch ();
	float offset;
	float slope;
	IOPCIDevice * NVCard;
	IOMemoryMap * nvio;
	float nv40_get_fanspeed();
	float nv43_get_fanspeed();
	float g84_get_fanspeed();
	float nv40_get_gpu_speed();
	float nv50_get_gpu_speed();
	float GetClock_nv40(int,unsigned int, unsigned int);
	float GetClock_nv50(int,unsigned int, unsigned int);
	int CalcSpeed_nv40(int,int,int,int,int,int);
	int CalcSpeed_nv50(int,int,int,int,int,int);
};
