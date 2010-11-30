/*
 * Copyright 2007 David F. Elliott.  All rights reserved.
 */

#ifndef __LIBSAIO_FAKE_EFI_H
#define __LIBSAIO_FAKE_EFI_H
#include "platform.h"
#include "pci.h"

/* Set up space for up to 10 configuration table entries */
#define MAX_CONFIGURATION_TABLE_ENTRIES 10

extern void setupFakeEfi(void);
extern void setupSystemType(void); 
extern inline uint64_t ptov64(uint32_t addr);
extern struct SMBEntryPoint * getSmbios(int which); // now cached
extern void setup_pci_devs(pci_dt_t *pci_dt);
extern uint64_t smbios_p;
extern void scan_cpu_DMI(void); //PlatformInfo_t *); //Slice

 
#endif /* !__LIBSAIO_FAKE_EFI_H */
