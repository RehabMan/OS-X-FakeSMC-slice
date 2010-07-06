/* add your code here */
#include <IOKit/IOLib.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <nvclock.h>
#include "FakeSMC.h"

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
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data){return kIOReturnInvalid;};
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data){return kIOReturnInvalid;};
};


class TemperatureSensor : public Binding {
public:
	TemperatureSensor(const char* key, const char* type, UInt8 size) : Binding(key, type, size) {};
	virtual IOReturn OnKeyRead(const char* key, char* data);
};

class FanSensor : public Binding {
public:
	FanSensor(const char* key, const char* type, UInt8 size) : Binding(key, type, size) {};
	virtual IOReturn OnKeyRead(const char* key, char* data);
};



class PTnVmon : public IOService {
	OSDeclareDefaultStructors(PTnVmon)
public:
	TemperatureSensor* tempSensor[MAX_CARDS];
	FanSensor* fanSensor[MAX_CARDS];
	virtual IOService*	probe	(IOService* provider, SInt32* score);
	virtual bool		start	(IOService* provider);
	virtual bool		init	(OSDictionary* properties=NULL);
	virtual void		free	();
	virtual	void		stop	(IOService* provider);
private:
	int					probe_devices	();
	int					max_card;
};

