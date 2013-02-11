/*
 *  HWInfo.h
 *  HWSensors
 *
 *  Created by Slice on 11.02.13.
 *  
 *
 */

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include <IOKit/IORegistryEntry.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOKitKeys.h>



class HWInfo : public IOService
{
  OSDeclareDefaultStructors(HWInfo)   
  
private:
	IOService*			fakeSMC;
	UInt32					SMCConfig;
	UInt8					  Mobile;
	char					  Platform[8];
	char					  PlatformB[8];
  UInt8           SMCRevision[6];
  bool            RPltset;
  bool            REVset;
  bool            EPCIset;
  bool            BEMBset;
	
public:
	virtual bool		    init(OSDictionary *properties=0);
	virtual IOService*	probe(IOService *provider, SInt32 *score);
  virtual bool		    start(IOService *provider);
	virtual void		    stop(IOService *provider);
	virtual void		    free(void);
	
	virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
};