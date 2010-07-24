
#ifndef __RADEON_PLUGIN__
#define __RADEON_PLUGIN__

#include <IOKit/IOLib.h>
#include <IOKit/pci/IOPCIDevice.h>
#include "FakeSMCBinding.h"
#include "ATICard.h"

class RadeonPlugin : public IOService {
	OSDeclareDefaultStructors(RadeonPlugin)
	friend class ATICard;
private:
	int				max_card;
	UInt32			chipID;	
	ATICard*		Card; 
public:
	virtual IOService*	probe	(IOService* provider, SInt32* score);
	virtual bool		start	(IOService* provider);
	virtual bool		init	(OSDictionary* properties=NULL);
	virtual void		free	();
	virtual	void		stop	(IOService* provider);
};

#endif
