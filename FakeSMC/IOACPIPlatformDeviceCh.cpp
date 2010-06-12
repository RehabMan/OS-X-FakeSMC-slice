/*
 *  IOACPIPlatformDeviceCh.cpp
 *  fakesmc
 *
 *  Created by Vladimir on 20.08.09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

//applesmc_ functions from qemu smc patch

#include "IOACPIPlatformDeviceCh.h"
/* data port used by Apple SMC */
//#include <i386/proc_reg.h>

#define APPLESMC_DATA_PORT	0x300
/* command/status port used by Apple SMC */
#define APPLESMC_CMD_PORT	0x304
#define APPLESMC_ERROR_CODE_PORT	0x31e
#define APPLESMC_NR_PORTS	32 /* 0x300-0x31f */
#define APPLESMC_MAX_DATA_LENGTH 32

#define APPLESMC_READ_CMD	0x10
#define APPLESMC_WRITE_CMD	0x11
#define APPLESMC_GET_KEY_BY_INDEX_CMD	0x12
#define APPLESMC_GET_KEY_TYPE_CMD	0x13
/*
static char osk[65] = "ourhardworkbythesewordsguardedpleasedontsteal(c)AppleComputerInc";
static char REV_key[] = {0x01, 0x30, 0x0f, 0x00, 0x00, 0x03};
static char MSSD_key[] = {0x03};
static char NTOK_key[] = {0x01};
static char SNUM_key[] = {0x01};
static char LSOF_key[] = {0x01};
static char LSSB_key[] = {0x01, 0x01};
 */

#define super IOService
OSDefineMetaClassAndStructors (IOACPIPlatformDeviceCh, IOACPIPlatformDevice)

void IOACPIPlatformDeviceCh::applesmc_io_cmd_writeb(void *opaque, uint32_t addr, uint32_t val)
{
    struct AppleSMCStatus *s = (struct AppleSMCStatus *)opaque;
//    IOLog("APPLESMC: CMD Write B: %#x = %#x\n", addr, val);
    switch(val) {
        case APPLESMC_READ_CMD:
            s->status = 0x0c;
            break;
		case APPLESMC_WRITE_CMD:
			 s->status = 0x0c;
			break;
		case APPLESMC_GET_KEY_BY_INDEX_CMD:
			s->status = 0x0c;
			break;
		case APPLESMC_GET_KEY_TYPE_CMD:
			s->status = 0x0c;
			break;
    }
    s->cmd = val;
    s->read_pos = 0;
    s->data_pos = 0;
	s->key_index = 0;
//	bzero(s->key_info, 6);
}

void IOACPIPlatformDeviceCh::applesmc_fill_data(struct AppleSMCStatus *s)
{
    struct AppleSMCData *d;
//	IOLog("FakeSMC: looking for key %c%c%c%c\n", s->key[0], s->key[1], s->key[2], s->key[3]);
    for( d = ASMCDKey; d; d = d->next) {
        uint32_t key_data = *((uint32_t*)d->key);
        uint32_t key_current = *((uint32_t*)s->key);
		if(key_data == key_current) {
//            IOLog("APPLESMC: Key matched (%s Len=%d Data=%s)\n", d->key, d->len, d->data);
			if(d->binding != NULL) d->binding->OnKeyRead(d->key, d->data);	
            memcpy(s->data, d->data, d->len);
            return;
        }
    }
	if(debug)
	IOLog("FakeSMC: key not found %c%c%c%c, length - %x\n", s->key[0], s->key[1], s->key[2], s->key[3],  s->data_len);
	s->status_1e=0x84;
}

char * IOACPIPlatformDeviceCh::applesmc_get_key_by_index(uint32_t index, struct AppleSMCStatus *s)
{
	struct AppleSMCData *d;
	uint32_t i=0;
	//	IOLog("looking for key %c%c%c%c\n", s->key[0], s->key[1], s->key[2], s->key[3]);
    for( d = ASMCDKey; d; i++, d = d->next) {
		if (i == index)
        return d->key;
    }
	if(debug)
	IOLog("FakeSMC: key by count %x is not found\n",index);
	s->status_1e=0x84;
	s->status = 0x00;
	return 0;
	
}

void IOACPIPlatformDeviceCh::applesmc_fill_info(struct AppleSMCStatus *s)
{
    struct AppleSMCData *d;
	
    for ( d = ASMCDKey; d; d = d->next) 
	{
        uint32_t key_data = *((uint32_t*)d->key);
        uint32_t key_current = *((uint32_t*)s->key);
		
        if (key_data == key_current) 
		{
			s->key_info[0] = d->len;
			s->key_info[5] = 0;
			
			int len = strlen(d->type);
			
			for (int i=0; i<4; i++)
			{
				if (i<len) 
				{
					s->key_info[i+1] = d->type[i];
				}
				else 
				{
					s->key_info[i+1] = 0;
				}
			}
			
            return;
        }
    }
	
	if(debug)
		IOLog("FakeSMC: key info not found %c%c%c%c, length - %x\n", s->key[0], s->key[1], s->key[2], s->key[3],  s->data_len);
	
	s->status_1e=0x84;
}

void IOACPIPlatformDeviceCh::applesmc_io_data_writeb(void *opaque, uint32_t addr, uint32_t val)
{
    struct AppleSMCStatus *s = (struct AppleSMCStatus *)opaque;
//    IOLog("APPLESMC: DATA Write B: %#x = %#x\n", addr, val);
    switch(s->cmd) {
        case APPLESMC_READ_CMD:
            if(s->read_pos < 4) {
                s->key[s->read_pos] = val;
                s->status = 0x04;
            } else if(s->read_pos == 4) {
                s->data_len = val;
                s->status = 0x05;
                s->data_pos = 0;
//                IOLog("APPLESMC: Key = %c%c%c%c Len = %d\n", s->key[0], s->key[1], s->key[2], s->key[3], val);
                applesmc_fill_data(s);
            }
            s->read_pos++;
            break;
		case APPLESMC_WRITE_CMD:
//			IOLog("FakeSMC: attempting to write(WRITE_CMD) to io port value %x ( %c )\n", val, val);
			if(s->read_pos < 4) {
                s->key[s->read_pos] = val;
                s->status = 0x04;
			} else if(s->read_pos == 4) {
				s->status = 0x05;
				s->data_pos=0;
				s->data_len = val;
//				IOLog("FakeSMC: System Tried to write Key = %c%c%c%c Len = %d\n", s->key[0], s->key[1], s->key[2], s->key[3], val);
			} else if( s->data_pos < s->data_len ) {
				s->data[s->data_pos] = val;
				s->data_pos++;
				s->status = 0x05;
				if(s->data_pos == s->data_len) {
					s->status = 0x00;
//					IOLog("FakeSMC: adding Key = %c%c%c%c Len = %d\n", s->key[0], s->key[1], s->key[2], s->key[3], s->data_len);
					SMCAddKey((const char*)s->key, s->data_len, (char*)s->data, 1);
					FixUpKeysNum();
					bzero(s->data, 255);
				}
			};
			s->read_pos++;
			break;
		case APPLESMC_GET_KEY_BY_INDEX_CMD:
//			IOLog("FakeSMC: System Tried to write GETKEYBYINDEX = %x (%c) at pos %x\n",val , val, s->read_pos);
			if(s->read_pos < 4) {
                s->key_index += val << (24 - s->read_pos * 8);
                s->status = 0x04;
				s->read_pos++;
			};
			if(s->read_pos == 4) {
				s->status = 0x05;
//				IOLog("FakeSMC: trying to find key by index %x\n", s->key_index);
				if(applesmc_get_key_by_index(s->key_index, s))
					bcopy(applesmc_get_key_by_index(s->key_index, s) , s->key, 4);
			}
			
			break;
		case APPLESMC_GET_KEY_TYPE_CMD:
//			IOLog("FakeSMC: System Tried to write GETKEYTYPE = %x (%c) at pos %x\n",val , val, s->read_pos);
			if(s->read_pos < 4) {
                s->key[s->read_pos] = val;
                s->status = 0x04;
            };
			s->read_pos++;
			if(s->read_pos == 4) {
				s->data_len = 6;  ///s->data_len = val ; ? val should be 6 here too
				s->status = 0x05;
				s->data_pos=0;
				applesmc_fill_info(s);
			}
			break;
    }
}

uint32_t IOACPIPlatformDeviceCh::applesmc_io_data_readb(void *opaque, uint32_t addr1)
{
	    struct AppleSMCStatus *s = (struct AppleSMCStatus *)opaque;
	    uint8_t retval = 0;
	    switch(s->cmd) {
			case APPLESMC_READ_CMD:
			    if(s->data_pos < s->data_len) {
			        retval = s->data[s->data_pos];
//			        IOLog("APPLESMC: READ_DATA[%d] = %#hhx\n", s->data_pos, retval);
			        s->data_pos++;
			        if(s->data_pos == s->data_len) {
			            s->status = 0x00;
						bzero(s->data, 255);
//			            IOLog("APPLESMC: EOF\n");
			         } else
			            s->status = 0x05;
			         }
				break;
			case APPLESMC_WRITE_CMD:
				IOLog("FakeSMC: attempting to read(WRITE_CMD) from io port");
				s->status = 0x00;
				break;
			case APPLESMC_GET_KEY_BY_INDEX_CMD:  ///shouldnt be here if status == 0
//				IOLog("FakeSMC:System Tried to read GETKEYBYINDEX = %x (%c) , at pos %d\n", retval, s->key[s->data_pos], s->key[s->data_pos], s->data_pos);
				if(s->status == 0) return 0; //sanity check
				if(s->data_pos < 4) {
					retval = s->key[s->data_pos];
					s->data_pos++;
				}
				if (s->data_pos == 4)
					s->status = 0x00;
				break;
			case APPLESMC_GET_KEY_TYPE_CMD:
//				IOLog("FakeSMC:System Tried to read GETKEYTYPE = %x , at pos %d\n", s->key_info[s->data_pos], s->data_pos);
				if(s->data_pos < s->data_len) {
			        retval = s->key_info[s->data_pos];
			        s->data_pos++;
			        if(s->data_pos == s->data_len) {
			            s->status = 0x00;
						bzero(s->key_info, 6);
						//			            IOLog("APPLESMC: EOF\n");
					} else
			            s->status = 0x05;
				}
				break;
				
    }
//    IOLog("APPLESMC: DATA Read b: %#x = %#x\n", addr1, retval);
    return retval;
}

uint32_t IOACPIPlatformDeviceCh::applesmc_io_cmd_readb(void *opaque, uint32_t addr1)
{
//		IOLog("APPLESMC: CMD Read B: %#x\n", addr1);
	    return ((struct AppleSMCStatus*)opaque)->status;
}

UInt32 IOACPIPlatformDeviceCh::ioRead32( UInt16 offset, IOMemoryMap * map )
{
    UInt32  value=0;
    UInt16  base = 0;
	
    if (map) base = map->getPhysicalAddress();
	
	IOLog("ioread32 called\n");
    return (value);
}

UInt16 IOACPIPlatformDeviceCh::ioRead16( UInt16 offset, IOMemoryMap * map )
{
    UInt16  value=0;
    UInt16  base = 0;
	
    if (map) base = map->getPhysicalAddress();
	
	IOLog("ioread16 called\n");

    return (value);
}

UInt8 IOACPIPlatformDeviceCh::ioRead8( UInt16 offset, IOMemoryMap * map )
{
    UInt8  value =0;
    UInt16  base = 0;
	struct AppleSMCStatus *s = (struct AppleSMCStatus *)stat;
//	IODelay(10);

    if (map) base = map->getPhysicalAddress();
	if((base+offset) == APPLESMC_DATA_PORT) value=applesmc_io_data_readb(stat, base+offset);
	if((base+offset) == APPLESMC_CMD_PORT) value=applesmc_io_cmd_readb(stat, base+offset);

    if((base+offset) == APPLESMC_ERROR_CODE_PORT)
	{
		if(s->status_1e != 0)
		{
			value = s->status_1e;
			s->status_1e = 0x00;
//			IOLog("generating error %x\n", value);
		}
		else value = 0x0;
	}
//	if(((base+offset) != APPLESMC_DATA_PORT) && ((base+offset) != APPLESMC_CMD_PORT)) IOLog("ioread8 to port %x.\n", base+offset);
	return (value);
}

void IOACPIPlatformDeviceCh::ioWrite32( UInt16 offset, UInt32 value,
									 IOMemoryMap * map )
{
    UInt16 base = 0;
	
    if (map) base = map->getPhysicalAddress();
	IOLog("iowrite32 called\n");

}

void IOACPIPlatformDeviceCh::ioWrite16( UInt16 offset, UInt16 value,
									 IOMemoryMap * map )
{
    UInt16 base = 0;
	
    if (map) base = map->getPhysicalAddress();
	IOLog("iowrite16 called\n");

}

void IOACPIPlatformDeviceCh::ioWrite8( UInt16 offset, UInt8 value,
									IOMemoryMap * map )
{
    UInt16 base = 0;
	IODelay(10);
    if (map) base = map->getPhysicalAddress();

	if((base+offset) == APPLESMC_DATA_PORT) applesmc_io_data_writeb(stat, base+offset, value);
	if((base+offset) == APPLESMC_CMD_PORT) applesmc_io_cmd_writeb(stat, base+offset,value);
	//    outb( base + offset, value );
//	if(((base+offset) != APPLESMC_DATA_PORT) && ((base+offset) != APPLESMC_CMD_PORT)) IOLog("iowrite8 to port %x.\n", base+offset);
}

void IOACPIPlatformDeviceCh::SMCSetup()
{
	stat= (ApleSMCStatus *) IOMalloc(sizeof(struct AppleSMCStatus));
	bzero((void*)stat, sizeof(struct AppleSMCStatus));
	debug = false;
	ASMCDKey = 0;
	/*
	SMCAddKey("REV ", 6, REV_key, 0);
	SMCAddKey("OSK0", 32, osk, 0);
	SMCAddKey("OSK1", 32, osk+32, 0);
	SMCAddKey("NATJ", 1, "\0", 0);
	SMCAddKey("MSSP", 1, "\0", 0);
	SMCAddKey("MSSD", 1, MSSD_key, 0);
	SMCAddKey("NTOK", 1, NTOK_key, 0);
	SMCAddKey("$Num", 1, SNUM_key, 0);
	SMCAddKey("LSOF", 1, LSOF_key, 0);
	SMCAddKey("LSSB", 2, LSSB_key, 0);
	 */
	FixUpKeysNum();
	interrupt_handler=0;
}

uint32_t IOACPIPlatformDeviceCh::GetKeysAmount()
{
	SMCData SMCkey=0;
	uint32_t keysno=0;
	for ( SMCkey = ASMCDKey; SMCkey; SMCkey = SMCkey->next )keysno++;
	return keysno;

}

void IOACPIPlatformDeviceCh::FixUpKeysNum()
{
	uint32_t keysno=0;
	uint32_t i = 1;
	
	if(FindSMCKey("#KEY")) i = 0;  //on first launch #KEY isnt present, so getkeysamount doesnt count it
	
	keysno = GetKeysAmount() + i;
	char NKEY_key[] = { keysno << 24, keysno << 16, keysno << 8, keysno };
	SMCAddKey("#KEY", 4, NKEY_key, 1);

}

//if replace_flag = 0 - existing key will not be overwrited, if != 0 - will be overwrited.
SMCData IOACPIPlatformDeviceCh::SMCAddKey(const char * keyname, uint8_t keylen, char * keydata, uint32_t replace_flag)
{
	return SMCAddKey(keyname, "", keylen, keydata, replace_flag, NULL);
}

SMCData IOACPIPlatformDeviceCh::SMCAddKey(const char * keyname, uint8_t keylen, char * keydata, uint32_t replace_flag, FakeSMCBinding* binding)
{
	return SMCAddKey(keyname, "", keylen, keydata, replace_flag, binding);
}

SMCData IOACPIPlatformDeviceCh::SMCAddKey(const char * keyname, const char * keytype, uint8_t keylen, char * keydata, uint32_t replace_flag)
{
	return SMCAddKey(keyname, keytype, keylen, keydata, replace_flag, NULL);
}

SMCData IOACPIPlatformDeviceCh::SMCAddKey(const char * keyname, const char * keytype, uint8_t keylen, char * keydata, uint32_t replace_flag, FakeSMCBinding* binding)
{
	SMCData SMCkey=0;
	SMCData PrevKey=0;
	
	SMCkey = FindSMCKey(keyname);
	
	if(SMCkey)
	{
		if(replace_flag != 0) 
		{
			if (strlen(keytype) > 0) bcopy(keytype, SMCkey->type, 5);
			SMCkey->len = keylen;
			IOFree(SMCkey->data, keylen);
			SMCkey->data = (char*) IOMalloc(keylen);
			bcopy(keydata, SMCkey->data,keylen);
			
			if (SMCkey->binding != NULL)
				SMCkey->binding->OnKeyWrite(keyname, SMCkey->data);
			
			if (binding) SMCkey->binding = binding;
			//			IOLog("FakeSMC: replacing key (%s Len=%d)\n", keyname, SMCkey->len);
		}
		return SMCkey;
	}
	
	for ( SMCkey = ASMCDKey; SMCkey; SMCkey = SMCkey->next )
		PrevKey = SMCkey;
	
	if(ASMCDKey == 0) 
	{
		SMCkey = (ASMCDKey = (AppleSMCData *)IOMalloc(sizeof(struct AppleSMCData)));
	}
	else 
	{
		SMCkey = (AppleSMCData *)IOMalloc(sizeof(struct AppleSMCData));
		PrevKey->next = SMCkey;
	}
	
	bzero((void*)SMCkey, sizeof(struct AppleSMCData));
	
	SMCkey->len = keylen;
	SMCkey->key = (char*) IOMalloc(5);
	SMCkey->type = (char*) IOMalloc(5); 
	SMCkey->data = (char*) IOMalloc(keylen);
	if (binding) SMCkey->binding = binding;	
	bzero(SMCkey->data, keylen);
	bcopy(keyname, SMCkey->key, 5); //size of key name is 4 chars + \0
	if (strlen(keytype) > 0)
	{
		bcopy(keytype, SMCkey->type, 5);
	}
	else 
	{
		switch (keylen) 
		{
			case 1:
				bcopy("ui8\0", SMCkey->type, 5);
				break;
			case 2:
				bcopy("ui16", SMCkey->type, 5);
				break;
			case 4:
				bcopy("ui32", SMCkey->type, 5);
				break;
			default:
				bcopy("ch8*", SMCkey->type, 5);
				break;
		}
	}
	
	if(keydata)
		bcopy(keydata, SMCkey->data,keylen);
	
	return SMCkey;
}

SMCData IOACPIPlatformDeviceCh::FindSMCKey(const char * keyname)
{
	struct AppleSMCData *d;
	for( d = ASMCDKey; d; d = d->next) {
        uint32_t key_data = *((uint32_t*)d->key);
        uint32_t key_current = *((uint32_t*)keyname);
        if(key_data == key_current) {
//			IOLog("FakeSMC: key already exists (%s Len=%d)\n", d->key, d->len);
            return d;
        }
    }
	return 0;
}

void IOACPIPlatformDeviceCh::SetDebug(bool debug_val)
{
	debug = debug_val;
}


IOReturn IOACPIPlatformDeviceCh::registerInterrupt(int source, OSObject *target,
								   IOInterruptAction handler,
								   void *refCon)
{
	interrupt_refcon = refCon;
	interrupt_target = target;
	interrupt_handler = handler;
	interrupt_source = source;
//	IOLog("register interrupt called for source %x\n", source);
	return kIOReturnSuccess;
}

IOReturn IOACPIPlatformDeviceCh::unregisterInterrupt(int source)
{
	return kIOReturnSuccess;
}

IOReturn IOACPIPlatformDeviceCh::getInterruptType(int source, int *interruptType)
{
	return kIOReturnSuccess;
}

IOReturn IOACPIPlatformDeviceCh::enableInterrupt(int source)
{
	return kIOReturnSuccess;
}

IOReturn IOACPIPlatformDeviceCh::disableInterrupt(int source)
{
	return kIOReturnSuccess;
}

IOReturn IOACPIPlatformDeviceCh::causeInterrupt(int source)
{
	if(interrupt_handler)
		interrupt_handler(interrupt_target, interrupt_refcon, this, interrupt_source);
	return kIOReturnSuccess;
}
