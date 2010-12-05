/*
 *  ACPIMonitor.h
 *  HWSensors
 *
 *  Created by mozo on 12/11/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include "IOKit/acpi/IOACPIPlatformDevice.h"
#include <IOKit/IOTimerEventSource.h>

class ACPIMonitor : public IOService
{
    OSDeclareDefaultStructors(ACPIMonitor)    
private:
	int						TCount;
	int						FCount;
	int						SMCx[30];
	UInt8					FanOffset;
	IOACPIPlatformDevice *	TZDevice;
	
	IOService*			fakeSMC;
	OSArray*			sensors;
	
	bool				addSensor(const char* key, const char* type, unsigned char size);
	int					addTachometer(const char* key, const char* caption);
	
protected:
	const char*			FanName[5];
	
public:
	virtual IOService*	probe(IOService *provider, SInt32 *score);
    virtual bool		start(IOService *provider);
	virtual bool		init(OSDictionary *properties=0);
	virtual void		free(void);
	virtual void		stop(IOService *provider);
	
	virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};
