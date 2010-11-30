/*
 * Copyright 2008 mackerintel
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

#include "libsaio.h"

extern void *new_dsdt;
extern uint64_t smbios_p;
//Slice - it's a shit to place a variable into header
/*
bool fix_restart;
uint64_t acpi10_p;
uint64_t acpi20_p;
uint64_t smbios_p;
int rsdplength;
void *new_dsdt=NULL;
*/
extern int setupAcpi();

extern EFI_STATUS addConfigurationTable();

extern EFI_GUID gEfiAcpiTableGuid;
extern EFI_GUID gEfiAcpi20TableGuid;

struct p_state 
{
	union 
	{
		uint16_t Control;
		struct 
		{
			uint8_t VID;	// Voltage ID
			uint8_t FID;	// Frequency ID
		};
	};
	
	uint8_t		CID;		// Compare ID
	uint32_t	Frequency;
};

#endif /* !__LIBSAIO_ACPI_PATCHER_H */
