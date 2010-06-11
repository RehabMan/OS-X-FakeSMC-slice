/*
 *  IOACPIPlatformDeviceCh.h
 *  fakesmc
 *
 *  Created by Vladimir on 20.08.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef IOACPIPlatformDeviceCh_h
#define IOACPIPlatformDeviceCh_h

#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>

#include "fakesmc.h"

struct AppleSMCStatus {
	uint8_t cmd;
	uint8_t status;
	uint8_t key[4];
	uint8_t read_pos;
	uint8_t data_len;
	uint8_t data_pos;
	uint8_t data[255];
	uint8_t charactic[4];
	uint8_t	status_1e;
	uint32_t key_index;
	uint8_t key_info[6];
};

struct AppleSMCData;

typedef struct AppleSMCData* SMCData;
typedef void (*OnKeyReadCallback)(const char*, char*);
typedef void (*OnKeyWriteCallback)(const char*, char*, bool*);

struct AppleSMCData {
	uint8_t len;
	char *key;
	char *type;
	char *data;
	FakeSMCPlugin* binding;
	SMCData next;
};


#include <IOKit/acpi/IOACPIPlatformDevice.h>

class IOACPIPlatformDeviceCh : public IOACPIPlatformDevice
{
    OSDeclareDefaultStructors( IOACPIPlatformDeviceCh )
	
	private:
	struct ApleSMCStatus *stat;
	OSObject *interrupt_target;
	struct AppleSMCData *ASMCDKey;
	IOInterruptAction interrupt_handler;
	void * interrupt_refcon;
	int interrupt_source;
	bool debug;
	
	virtual void applesmc_io_cmd_writeb(void *opaque, uint32_t addr, uint32_t val);
	
	virtual void applesmc_fill_data(struct AppleSMCStatus *s);
	
	virtual void applesmc_io_data_writeb(void *opaque, uint32_t addr, uint32_t val);
	
	virtual uint32_t applesmc_io_data_readb(void *opaque, uint32_t addr1);
	
	virtual uint32_t applesmc_io_cmd_readb(void *opaque, uint32_t addr1);
	
	virtual char * applesmc_get_key_by_index(uint32_t index, struct AppleSMCStatus *s);
	
	virtual void applesmc_fill_info(struct AppleSMCStatus *s);
	
	public:
    // I/O space helpers
	
    virtual void     ioWrite32( UInt16 offset, UInt32 value,
							   IOMemoryMap * map = 0 );
	
    virtual void     ioWrite16( UInt16 offset, UInt16 value,
							   IOMemoryMap * map = 0 );
	
    virtual void     ioWrite8(  UInt16 offset, UInt8 value,
							  IOMemoryMap * map = 0 );
	
    virtual UInt32   ioRead32( UInt16 offset, IOMemoryMap * map = 0 );
	
    virtual UInt16   ioRead16( UInt16 offset, IOMemoryMap * map = 0 );
	
    virtual UInt8    ioRead8(  UInt16 offset, IOMemoryMap * map = 0 );
	
	virtual IOReturn registerInterrupt(int source, OSObject *target,
									   IOInterruptAction handler,
									   void *refCon = 0);
	
    virtual IOReturn unregisterInterrupt(int source);
	
    virtual IOReturn getInterruptType(int source, int *interruptType);
	
    virtual IOReturn enableInterrupt(int source);
	
    virtual IOReturn disableInterrupt(int source);
	
	virtual IOReturn causeInterrupt(int source);
	
	virtual void	SMCSetup();
	
	virtual SMCData SMCAddKey(const char * keyname, uint8_t keylen, char * keydata, uint32_t replace_flag); //returns a pointer to key struct, so we can modify it later 
	virtual SMCData SMCAddKey(const char * keyname, uint8_t keylen, char * keydata, uint32_t replace_flag, FakeSMCPlugin* binding);
	virtual SMCData SMCAddKey(const char * keyname, const char * keytype, uint8_t keylen, char * keydata, uint32_t replace_flag);
	virtual SMCData SMCAddKey(const char * keyname, const char * keytype, uint8_t keylen, char * keydata, uint32_t replace_flag, FakeSMCPlugin* binding);

	
	virtual SMCData FindSMCKey(const char * keyname);
	
	virtual uint32_t GetKeysAmount(void);
	
	virtual void FixUpKeysNum(void);
	
	virtual void SetDebug(bool debug_val);
		
};

#endif
