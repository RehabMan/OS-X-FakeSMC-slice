#include <IOKit/IOLib.h>
#include <IOKit/pci/IOPCIDevice.h>
#include "FakeSMCBinding.h"
#include "radeon_chipinfo_gen.h"

#define MAX_CARDS 2
#define GPU_OVERCLOCKING (1<<0)
#define MEM_OVERCLOCKING (1<<1)
#define COOLBITS_OVERCLOCKING (1<<2)
#define PIPELINE_MODDING (1<<3)
#define GPU_FANSPEED_MONITORING (1<<4) /* Fanspeed monitoring based on fan voltage */
#define BOARD_TEMP_MONITORING (1<<5) /* Board temperature */
#define GPU_TEMP_MONITORING (1<<6) /* Internal GPU temperature */
#define I2C_FANSPEED_MONITORING (1<<7) /* Fanspeed monitoring using a i2c sensor chip */
#define I2C_AUTOMATIC_FANSPEED_CONTROL (1<<8) /* The sensor supports automatic fanspeed control */
#define SMARTDIMMER (1<<9) /* Smartdimmer support for mobile GPUs */
#define GPU_ID_MODDING (1<<10) /* PCI id modding is supported on this board */


class Binding : public FakeSMCBinding {
protected:
	char*			m_Key;
public:
	Binding*		Next;	
	Binding(){};	
	Binding(const char* key, const char* type, UInt8 size) {
		//IOLog("Binding key %s\n", key);		
		m_Key = (char*)IOMalloc(5);
		bcopy(key, m_Key, 5);		
		char* value = (char*)IOMalloc(size);
		FakeSMCAddKey(key, type, size, value, this);
		IOFree(value, size);
	};
	
	~Binding() {
		if (m_Key) {
			//IOLog("Removing key %s binding\n", m_Key);			
			IOFree(m_Key, 5);
			FakeSMCRemoveKeyBinding(m_Key);
		}
	};
	const char* GetKey() {return m_Key;};
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data)
		{return kIOReturnInvalid;};
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data)
		{return kIOReturnInvalid;};
};


class RadeonPlugin : public IOService {
	OSDeclareDefaultStructors(RadeonPlugin)
private:
	volatile		UInt8* mmio_base;
	RADEONCardInfo*	rinfo;
	UInt32			chipID;
	IOPCIDevice *	VCard;
	IOMemoryMap *	mmio;
	UInt32			Caps;
	int				max_card;
	UInt32			tReg;
public:
	TemperatureSensor* tempSensor[MAX_CARDS];
	TemperatureSensor* boardSensor[MAX_CARDS];
	FanSensor* fanSensor[MAX_CARDS];
	virtual IOService*	probe	(IOService* provider, SInt32* score);
	virtual bool		start	(IOService* provider);
	virtual bool		init	(OSDictionary* properties=NULL);
	virtual void		free	();
	virtual	void		stop	(IOService* provider);
private:
//	int					probe_devices	();
	void				getRadeonInfo	();
	void				setup_R6xx		(int N);
	void				setup_R7xx		(int N);

};

