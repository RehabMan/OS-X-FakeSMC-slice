
/*
 * Copyright 2007 David F. Elliott.	 All rights reserved.
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "efi.h"
#include "acpi.h"
#include "fake_efi.h"
#include "efi_tables.h"
#include "platform.h"
#include "acpi_patcher.h"
#include "smbios_patcher.h"
#include "device_inject.h"
#include "convert.h"
#include "pci.h"
#include "sl.h"
#include "mem.h"
//#include "SMBIOS.h"  //Slice -?

extern struct SMBEntryPoint * getSmbios(int which); // now cached
extern void setup_pci_devs(pci_dt_t *pci_dt);

/*
 * Modern Darwin kernels require some amount of EFI because Apple machines all
 * have EFI.  Modifying the kernel source to not require EFI is of course
 * possible but would have to be maintained as a separate patch because it is
 * unlikely that Apple wishes to add legacy support to their kernel.
 *
 * As you can see from the Apple-supplied code in bootstruct.c, it seems that
 * the intention was clearly to modify this booter to provide EFI-like structures
 * to the kernel rather than modifying the kernel to handle non-EFI stuff.	 This
 * makes a lot of sense from an engineering point of view as it means the kernel
 * for the as yet unreleased EFI-only Macs could still be booted by the non-EFI
 * DTK systems so long as the kernel checked to ensure the boot tables were
 * filled in appropriately.	 Modern xnu requires a system table and a runtime
 * services table and performs no checks whatsoever to ensure the pointers to
 * these tables are non-NULL.	Therefore, any modern xnu kernel will page fault
 * early on in the boot process if the system table pointer is zero.
 *
 * Even before that happens, the tsc_init function in modern xnu requires the FSB
 * Frequency to be a property in the /efi/platform node of the device tree or else
 * it panics the bootstrap process very early on.
 *
 * As of this writing, the current implementation found here is good enough
 * to make the currently available xnu kernel boot without modification on a
 * system with an appropriate processor.  With a minor source modification to
 * the tsc_init function to remove the explicit check for Core or Core 2
 * processors the kernel can be made to boot on other processors so long as
 * the code can be executed by the processor and the machine contains the
 * necessary hardware.
 */


 static const char const FIRMWARE_REVISION_PROP[] = "firmware-revision";
 static const char const FIRMWARE_ABI_PROP[] = "firmware-abi";
 static const char const FIRMWARE_VENDOR_PROP[] = "firmware-vendor";
 static const char const FIRMWARE_ABI_32_PROP_VALUE[] = "EFI32";
 static const char const FIRMWARE_ABI_64_PROP_VALUE[] = "EFI64";
 static const char const PLATFORM_UUID[] = "platform-uuid"; 
 static const char const SYSTEM_ID_PROP[] = "system-id";
 static const char const SYSTEM_SERIAL_PROP[] = "SystemSerialNumber";
 static const char const SYSTEM_TYPE_PROP[] = "system-type";
 static const char const MODEL_PROP[] = "Model";

#define kBL_GLOBAL_NVRAM_GUID "8BE4DF61-93CA-11D2-AA0D-00E098032B8C"

// Check if a system supports CSM legacy mode
#define kBL_APPLE_VENDOR_NVRAM_GUID "4D1EDE05-38C7-4A6A-9CC6-4BCCA8B38C14"

/* From Foundation/Efi/Guid/Smbios/SmBios.c */
EFI_GUID const	gEfiSmbiosTableGuid = EFI_SMBIOS_TABLE_GUID;

#define SMBIOS_RANGE_START		0x000F0000
#define SMBIOS_RANGE_END		0x000FFFFF

/* '_SM_' in little endian: */
#define SMBIOS_ANCHOR_UINT32_LE 0x5f4d535f

#define EFI_ACPI_TABLE_GUID \
{ \
0xeb9d2d30, 0x2d88, 0x11d3, 0x9a, 0x16, {0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d } \
}

#define EFI_ACPI_20_TABLE_GUID \
{ \
0x8868e871, 0xe4f1, 0x11d3, 0xbc, 0x22, {0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } \
}

EFI_GUID gEfiAcpiTableGuid = EFI_ACPI_TABLE_GUID;
EFI_GUID gEfiAcpi20TableGuid = EFI_ACPI_20_TABLE_GUID;



/*
 * Must be called AFTER setup Acpi because we need to take care of correct
 * facp content to reflect in ioregs
 */
void setupSystemType()
{
	Node *node;
    node = DT__FindNode("/", false);
    if (node == 0) {
        return;
    }
	
	/* Export system-type */

	verbose("Using system-type=0x%02x\n", Platform.Type);
	DT__AddProperty(node, SYSTEM_TYPE_PROP, sizeof(Platform.Type), &Platform.Type);
	
}

/*==========================================================================
 * Utility function to make a device tree string from an EFI_GUID
 */

static inline char * mallocStringForGuid(EFI_GUID const *pGuid)
{
	char *string = malloc(37);
	efi_guid_unparse_upper(pGuid, string);
	return string;
}

/*==========================================================================
 * Function to map 32 bit physical address to 64 bit virtual address
 */

static uint64_t ptov64(uint32_t addr)
{
	return ((uint64_t)addr | 0xFFFFFF8000000000ULL);
}

/*==========================================================================
 * Fake EFI implementation
 */

/* Identify ourselves as the EFI firmware vendor */
static EFI_CHAR16 const FIRMWARE_VENDOR[] = {'C','h','a','m','e','l','e','o','n','_','2','.','5','S', 0};
static EFI_UINT32 const FIRMWARE_REVISION = 0x0001000a; 
static EFI_UINT32 const FIRMWARE_FEATURE_MASK = 0x000003FF;
static EFI_UINT32 const STATIC_ZERO = 0;

/* Default platform system_id (fix by IntVar) */
//static EFI_CHAR8 const SYSTEM_ID[] = "0123456789ABCDEF"; //random value gen by uuidgen

/* Just a ret instruction */
static uint8_t const VOIDRET_INSTRUCTIONS[] = {0xc3};

/* movl $0x80000003,%eax; ret */
static uint8_t const UNSUPPORTEDRET_INSTRUCTIONS[] = {0xb8, 0x03, 0x00, 0x00, 0x80, 0xc3};
//#define SYSTEM_ID_DEFAULT {0x41, 0x73, 0x65, 0x72, 0x65, 0x42, 0x4c, 0x4e, 0x66, 0x75, 0x63, 0x6b, 0x45, 0x46, 0x49, 0x58}
EFI_GUID const SYSTEM_ID_DEFAULT = {0x72657341, 0x4265, 0x4e4C, 0x66, 0x75, {0x63, 0x6b, 0x45, 0x46, 0x49, 0x58}};
EFI_SYSTEM_TABLE_32 *gST32 = NULL;
EFI_SYSTEM_TABLE_64 *gST64 = NULL;
Node *gEfiConfigurationTableNode = NULL;

extern EFI_STATUS addConfigurationTable(EFI_GUID const *pGuid, void *table, char const *alias)
{
	EFI_UINTN i = 0;
	
	//Azi: as is, cpu's with em64t will use EFI64 on pre 10.6 systems,
	// wich seems to cause no problem. In case it does, force i386 arch.
	if (archCpuType == CPU_TYPE_I386)
	{
		i = gST32->NumberOfTableEntries;
	}
	else
	{
		i = gST64->NumberOfTableEntries;
	}
	
	// We only do adds, not modifications and deletes like InstallConfigurationTable
	if (i >= MAX_CONFIGURATION_TABLE_ENTRIES)
		stop("Ran out of space for configuration tables.  Increase the reserved size in the code.\n");
	
	if (pGuid == NULL)
		return EFI_INVALID_PARAMETER;
	
	if (table != NULL)
	{
		// FIXME
		//((EFI_CONFIGURATION_TABLE_64 *)gST->ConfigurationTable)[i].VendorGuid = *pGuid;
		//((EFI_CONFIGURATION_TABLE_64 *)gST->ConfigurationTable)[i].VendorTable = (EFI_PTR64)table;
		
		//++gST->NumberOfTableEntries;
		
		Node *tableNode = DT__AddChild(gEfiConfigurationTableNode, mallocStringForGuid(pGuid));
		
		// Use the pointer to the GUID we just stuffed into the system table
		DT__AddProperty(tableNode, "guid", sizeof(EFI_GUID), (void*)pGuid);
		
		// The "table" property is the 32-bit (in our implementation) physical address of the table
		DT__AddProperty(tableNode, "table", sizeof(void*) * 2, table);
		
		// Assume the alias pointer is a global or static piece of data
		if (alias != NULL)
			DT__AddProperty(tableNode, "alias", strlen(alias)+1, (char*)alias);
		
		return EFI_SUCCESS;
	}
	return EFI_UNSUPPORTED;
}

//Azi: crc32 done in place, on the cases were it wasn't.
/*static inline void fixupEfiSystemTableCRC32(EFI_SYSTEM_TABLE_64 *efiSystemTable)
{
	efiSystemTable->Hdr.CRC32 = 0;
	efiSystemTable->Hdr.CRC32 = crc32(0L, efiSystemTable, efiSystemTable->Hdr.HeaderSize);
}*/

/*
 * What we do here is simply allocate a fake EFI system table and a fake EFI
 * runtime services table.
 *
 * Because we build against modern headers with kBootArgsRevision 4 we
 * also take care to set efiMode = 32.
 */

void setupEfiTables32(void)
{
	// We use the fake_efi_pages struct so that we only need to do one kernel
	// memory allocation for all needed EFI data.  Otherwise, small allocations
	// like the FIRMWARE_VENDOR string would take up an entire page.
	// NOTE WELL: Do NOT assume this struct has any particular layout within itself.
	// It is absolutely not intended to be publicly exposed anywhere
	// We say pages (plural) although right now we are well within the 1 page size
	// and probably will stay that way.
	struct fake_efi_pages
	{
		EFI_SYSTEM_TABLE_32 efiSystemTable;
		EFI_RUNTIME_SERVICES_32 efiRuntimeServices;
		EFI_CONFIGURATION_TABLE_32 efiConfigurationTable[MAX_CONFIGURATION_TABLE_ENTRIES];
		EFI_CHAR16 firmwareVendor[sizeof(FIRMWARE_VENDOR)/sizeof(EFI_CHAR16)];
		uint8_t voidret_instructions[sizeof(VOIDRET_INSTRUCTIONS)/sizeof(uint8_t)];
		uint8_t unsupportedret_instructions[sizeof(UNSUPPORTEDRET_INSTRUCTIONS)/sizeof(uint8_t)];
	};
	
	struct fake_efi_pages *fakeEfiPages = (struct fake_efi_pages*)AllocateKernelMemory(sizeof(struct fake_efi_pages));
	
	// Zero out all the tables in case fields are added later
	bzero(fakeEfiPages, sizeof(struct fake_efi_pages));
	
	// --------------------------------------------------------------------
	// Initialize some machine code that will return EFI_UNSUPPORTED for
	// functions returning int and simply return for void functions.
	memcpy(fakeEfiPages->voidret_instructions, VOIDRET_INSTRUCTIONS, sizeof(VOIDRET_INSTRUCTIONS));
	memcpy(fakeEfiPages->unsupportedret_instructions, UNSUPPORTEDRET_INSTRUCTIONS, sizeof(UNSUPPORTEDRET_INSTRUCTIONS));
	
	// --------------------------------------------------------------------
	// System table
	EFI_SYSTEM_TABLE_32 *efiSystemTable = gST32 = &fakeEfiPages->efiSystemTable;
	efiSystemTable->Hdr.Signature = EFI_SYSTEM_TABLE_SIGNATURE;
	efiSystemTable->Hdr.Revision = EFI_SYSTEM_TABLE_REVISION;
	efiSystemTable->Hdr.HeaderSize = sizeof(EFI_SYSTEM_TABLE_32);
	efiSystemTable->Hdr.CRC32 = 0; // Initialize to zero and then do CRC32
	efiSystemTable->Hdr.Reserved = 0;
	
	efiSystemTable->FirmwareVendor = (EFI_PTR32)&fakeEfiPages->firmwareVendor;
	memcpy(fakeEfiPages->firmwareVendor, FIRMWARE_VENDOR, sizeof(FIRMWARE_VENDOR));
	efiSystemTable->FirmwareRevision = FIRMWARE_REVISION;
	
	// XXX: We may need to have basic implementations of ConIn/ConOut/StdErr
	// The EFI spec states that all handles are invalid after boot services have been
	// exited so we can probably get by with leaving the handles as zero.
	efiSystemTable->ConsoleInHandle = 0;
	efiSystemTable->ConIn = 0;
	
	efiSystemTable->ConsoleOutHandle = 0;
	efiSystemTable->ConOut = 0;
	
	efiSystemTable->StandardErrorHandle = 0;
	efiSystemTable->StdErr = 0;
	efiSystemTable->RuntimeServices = (EFI_PTR32)&fakeEfiPages->efiRuntimeServices;
	
	// According to the EFI spec, BootServices aren't valid after the
	// boot process is exited so we can probably do without it.
	// Apple didn't provide a definition for it in pexpert/i386/efi.h
	// so I'm guessing they don't use it.
	efiSystemTable->BootServices = 0;
	
	efiSystemTable->NumberOfTableEntries = 0;
	efiSystemTable->ConfigurationTable = (EFI_PTR32)fakeEfiPages->efiConfigurationTable;
	
	// We're done. Now CRC32 the thing so the kernel will accept it.
	// Must be initialized to zero before CRC32, done above.
	gST32->Hdr.CRC32 = crc32(0L, gST32, gST32->Hdr.HeaderSize);
	
	// --------------------------------------------------------------------
	// Runtime services
	EFI_RUNTIME_SERVICES_32 *efiRuntimeServices = &fakeEfiPages->efiRuntimeServices;
	efiRuntimeServices->Hdr.Signature = EFI_RUNTIME_SERVICES_SIGNATURE;
	efiRuntimeServices->Hdr.Revision = EFI_RUNTIME_SERVICES_REVISION;
	efiRuntimeServices->Hdr.HeaderSize = sizeof(EFI_RUNTIME_SERVICES_32);
	efiRuntimeServices->Hdr.CRC32 = 0;
	efiRuntimeServices->Hdr.Reserved = 0;
	
	// There are a number of function pointers in the efiRuntimeServices table.
	// These are the Foundation (e.g. core) services and are expected to be present on
	// all EFI-compliant machines.	Some kernel extensions (notably AppleEFIRuntime)
	// will call these without checking to see if they are null.
	//
	// We don't really feel like doing an EFI implementation in the bootloader
	// but it is nice if we can at least prevent a complete crash by
	// at least providing some sort of implementation until one can be provided
	// nicely in a kext.
	void (*voidret_fp)() = (void*)fakeEfiPages->voidret_instructions;
	void (*unsupportedret_fp)() = (void*)fakeEfiPages->unsupportedret_instructions;
	efiRuntimeServices->GetTime = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->SetTime = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->GetWakeupTime = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->SetWakeupTime = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->SetVirtualAddressMap = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->ConvertPointer = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->GetVariable = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->GetNextVariableName = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->SetVariable = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->GetNextHighMonotonicCount = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->ResetSystem = (EFI_PTR32)voidret_fp;
	
	// We're done.	Now CRC32 the thing so the kernel will accept it
	efiRuntimeServices->Hdr.CRC32 = crc32(0L, efiRuntimeServices, efiRuntimeServices->Hdr.HeaderSize);
	
	// --------------------------------------------------------------------
	// Finish filling in the rest of the boot args that we need.
	bootArgs->efiSystemTable = (uint32_t)efiSystemTable;
	bootArgs->efiMode = kBootArgsEfiMode32;
	
	// The bootArgs structure as a whole is bzero'd so we don't need to fill in
	// things like efiRuntimeServices* and what not.
	//
	// In fact, the only code that seems to use that is the hibernate code so it
	// knows not to save the pages.	 It even checks to make sure its nonzero.
}

void setupEfiTables64(void)
{
	struct fake_efi_pages
	{
		EFI_SYSTEM_TABLE_64 efiSystemTable;
		EFI_RUNTIME_SERVICES_64 efiRuntimeServices;
		EFI_CONFIGURATION_TABLE_64 efiConfigurationTable[MAX_CONFIGURATION_TABLE_ENTRIES];
		EFI_CHAR16 firmwareVendor[sizeof(FIRMWARE_VENDOR)/sizeof(EFI_CHAR16)];
		uint8_t voidret_instructions[sizeof(VOIDRET_INSTRUCTIONS)/sizeof(uint8_t)];
		uint8_t unsupportedret_instructions[sizeof(UNSUPPORTEDRET_INSTRUCTIONS)/sizeof(uint8_t)];
	};
	
	struct fake_efi_pages *fakeEfiPages = (struct fake_efi_pages*)AllocateKernelMemory(sizeof(struct fake_efi_pages));
	
	// Zero out all the tables in case fields are added later
	bzero(fakeEfiPages, sizeof(struct fake_efi_pages));
	
	// --------------------------------------------------------------------
	// Initialize some machine code that will return EFI_UNSUPPORTED for
	// functions returning int and simply return for void functions.
	memcpy(fakeEfiPages->voidret_instructions, VOIDRET_INSTRUCTIONS, sizeof(VOIDRET_INSTRUCTIONS));
	memcpy(fakeEfiPages->unsupportedret_instructions, UNSUPPORTEDRET_INSTRUCTIONS, sizeof(UNSUPPORTEDRET_INSTRUCTIONS));
	
	// --------------------------------------------------------------------
	// System table
	EFI_SYSTEM_TABLE_64 *efiSystemTable = gST64 = &fakeEfiPages->efiSystemTable;
	efiSystemTable->Hdr.Signature = EFI_SYSTEM_TABLE_SIGNATURE;
	efiSystemTable->Hdr.Revision = EFI_SYSTEM_TABLE_REVISION;
	efiSystemTable->Hdr.HeaderSize = sizeof(EFI_SYSTEM_TABLE_64);
	efiSystemTable->Hdr.CRC32 = 0; // Initialize to zero and then do CRC32
	efiSystemTable->Hdr.Reserved = 0;
	
	efiSystemTable->FirmwareVendor = ptov64((EFI_PTR32)&fakeEfiPages->firmwareVendor);
	memcpy(fakeEfiPages->firmwareVendor, FIRMWARE_VENDOR, sizeof(FIRMWARE_VENDOR));
	efiSystemTable->FirmwareRevision = FIRMWARE_REVISION;
	
	// XXX: We may need to have basic implementations of ConIn/ConOut/StdErr
	// The EFI spec states that all handles are invalid after boot services have been
	// exited so we can probably get by with leaving the handles as zero.
	efiSystemTable->ConsoleInHandle = 0;
	efiSystemTable->ConIn = 0;
	
	efiSystemTable->ConsoleOutHandle = 0;
	efiSystemTable->ConOut = 0;
	
	efiSystemTable->StandardErrorHandle = 0;
	efiSystemTable->StdErr = 0;
	
	efiSystemTable->RuntimeServices = ptov64((EFI_PTR32)&fakeEfiPages->efiRuntimeServices);
	// According to the EFI spec, BootServices aren't valid after the
	// boot process is exited so we can probably do without it.
	// Apple didn't provide a definition for it in pexpert/i386/efi.h
	// so I'm guessing they don't use it.
	efiSystemTable->BootServices = 0;
	
	efiSystemTable->NumberOfTableEntries = 0;
	efiSystemTable->ConfigurationTable = ptov64((EFI_PTR32)fakeEfiPages->efiConfigurationTable);
	
	// We're done.	Now CRC32 the thing so the kernel will accept it
	gST64->Hdr.CRC32 = crc32(0L, gST64, gST64->Hdr.HeaderSize);
	
	// --------------------------------------------------------------------
	// Runtime services
	EFI_RUNTIME_SERVICES_64 *efiRuntimeServices = &fakeEfiPages->efiRuntimeServices;
	efiRuntimeServices->Hdr.Signature = EFI_RUNTIME_SERVICES_SIGNATURE;
	efiRuntimeServices->Hdr.Revision = EFI_RUNTIME_SERVICES_REVISION;
	efiRuntimeServices->Hdr.HeaderSize = sizeof(EFI_RUNTIME_SERVICES_64);
	efiRuntimeServices->Hdr.CRC32 = 0;
	efiRuntimeServices->Hdr.Reserved = 0;
	
	// There are a number of function pointers in the efiRuntimeServices table.
	// These are the Foundation (e.g. core) services and are expected to be present on
	// all EFI-compliant machines.	Some kernel extensions (notably AppleEFIRuntime)
	// will call these without checking to see if they are null.
	//
	// We don't really feel like doing an EFI implementation in the bootloader
	// but it is nice if we can at least prevent a complete crash by
	// at least providing some sort of implementation until one can be provided
	// nicely in a kext.
	
	void (*voidret_fp)() = (void*)fakeEfiPages->voidret_instructions;
	void (*unsupportedret_fp)() = (void*)fakeEfiPages->unsupportedret_instructions;
	efiRuntimeServices->GetTime = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->SetTime = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->GetWakeupTime = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->SetWakeupTime = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->SetVirtualAddressMap = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->ConvertPointer = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->GetVariable = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->GetNextVariableName = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->SetVariable = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->GetNextHighMonotonicCount = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->ResetSystem = ptov64((EFI_PTR32)voidret_fp);
	
	// We're done.	Now CRC32 the thing so the kernel will accept it
	efiRuntimeServices->Hdr.CRC32 = crc32(0L, efiRuntimeServices, efiRuntimeServices->Hdr.HeaderSize);
	
	// --------------------------------------------------------------------
	// Finish filling in the rest of the boot args that we need.
	bootArgs->efiSystemTable = (uint32_t)efiSystemTable;
	bootArgs->efiMode = kBootArgsEfiMode64;
	
	// The bootArgs structure as a whole is bzero'd so we don't need to fill in
	// things like efiRuntimeServices* and what not.
	//
	// In fact, the only code that seems to use that is the hibernate code so it
	// knows not to save the pages.	 It even checks to make sure its nonzero.
}

/*
 * In addition to the EFI tables there is also the EFI device tree node.
 * In particular, we need /efi/platform to have an FSBFrequency key. Without it,
 * the tsc_init function will panic very early on in kernel startup, before
 * the console is available.
 */

/*==========================================================================
 * FSB Frequency detection
 */

/* These should be const but DT__AddProperty takes char* */
static const char const TSC_Frequency_prop[] = "TSCFrequency";
static const char const FSB_Frequency_prop[] = "FSBFrequency";
static const char const CPU_Frequency_prop[] = "CPUFrequency";

/*==========================================================================
 * SMBIOS
 *
 *
 * Get an smbios option string option to convert to EFI_CHAR16 string
 */

static EFI_CHAR16* getSmbiosChar16(const char * key, size_t* len)
{
	const char	*src = getStringForKey(key, &bootInfo->smbiosConfig);
	EFI_CHAR16*	 dst = 0;
	size_t		 i = 0;
	
	if (!key || !(*key) || !len || !src) return 0;
	
	*len = strlen(src);
	dst = (EFI_CHAR16*) malloc( ((*len)+1) * 2 );
	for (; i < (*len); i++)	 dst[i] = src[i];
	dst[(*len)] = '\0';
	*len = ((*len)+1)*2; // return the CHAR16 bufsize including zero terminated CHAR16
	return dst;
}

/*
 * Get the SystemID from the bios dmi info
 */

static	EFI_GUID * getSmbiosUUID()
{
	static EFI_GUID		 uuid;
	int						 i, isZero, isOnes;
//	struct SMBEntryPoint	*smbios;
	SMBByte					*p;
	
//	smbios = getSmbios(SMBIOS_PATCHED); // checks for _SM_ anchor and table header checksum
//	if (smbios==NULL) return 0; // getSmbios() return a non null value if smbios is found
	
	p = (SMBByte*) FindFirstDmiTableOfType(1, 0x19); // Type 1: (3.3.2) System Information
	if (p==NULL) return NULL;
	
	verbose("Found SMBIOS System Information Table 1\n");
	p += 8;
	
	for (i=0, isZero=1, isOnes=1; i<UUID_LEN; i++)
	{
		if (p[i] != 0x00) isZero = 0;
		if (p[i] != 0xff) isOnes = 0;
	}
	
	if (isZero || isOnes) // empty or setable means: no uuid present
	{
		verbose("No UUID present in SMBIOS System Information Table\n");
		return 0;
	}
	
	memcpy((void*)&uuid, p, UUID_LEN);
	return &uuid;
}

/*
 * return a binary UUID value from the overriden SystemID and SMUUID if found, 
 * or from the bios if not, or from a fixed value if no bios value is found 
 */

static EFI_GUID* getSystemID()
{
	EFI_GUID*	ret = NULL;
	// unable to determine UUID for host. Error: 35 fix
	// Rek: new SMsystemid option conforming to smbios notation standards, this option should
	// belong to smbios config only ...
	const char *sysId = getStringForKey(kSystemID, &bootInfo->bootConfig);
	if (sysId) {
		ret = (EFI_GUID*)getUUIDFromString(sysId);
	}
	
	if (!ret) // try bios dmi info UUID extraction
	{
		ret = getSmbiosUUID();
		sysId = 0;
	}
	
	if (!ret) {		
// no bios dmi UUID available, set a fixed value for system-id
//		ret=getUUIDFromString((sysId = (const char*) SYSTEM_ID));
		ret = (EFI_GUID*)&SYSTEM_ID_DEFAULT;
	}
	
	verbose("Customizing SystemID with : %s\n", getStringFromUUID(ret)); // apply a nice formatting to the displayed output
	
	return ret;
}

void setupEfiDeviceTree(void)
{
	EFI_GUID*	ret = 0;
	EFI_CHAR16*	ret16 = 0;
	size_t		len = 0;
	Node*		node;
//Slice
#if 1 //NOTYET	
	char		bootName[128];
	
	char*		ffName;
	uint8_t *	FirmwareFeatures;
	char*		ffmName;
	char*		boName;
	char*		bnName;

	uint16_t	bootOptionNumber = 0;
#endif	
//
	node = DT__FindNode("/", false);
	
	if (node == 0) stop("Couldn't get root node");
	
	// We could also just do DT__FindNode("/efi/platform", true)
	// But I think eventually we want to fill stuff in the efi node
	// too so we might as well create it so we have a pointer for it too.
	node = DT__AddChild(node, "efi");
	
	if (archCpuType == CPU_TYPE_I386)
	{
		DT__AddProperty(node, FIRMWARE_ABI_PROP, sizeof(FIRMWARE_ABI_32_PROP_VALUE), (char*)FIRMWARE_ABI_32_PROP_VALUE);
	}
	else
	{
		DT__AddProperty(node, FIRMWARE_ABI_PROP, sizeof(FIRMWARE_ABI_64_PROP_VALUE), (char*)FIRMWARE_ABI_64_PROP_VALUE);
	}
	
	DT__AddProperty(node, FIRMWARE_REVISION_PROP, sizeof(FIRMWARE_REVISION), (EFI_UINT32*)&FIRMWARE_REVISION);
	DT__AddProperty(node, FIRMWARE_VENDOR_PROP, sizeof(FIRMWARE_VENDOR), (EFI_CHAR16*)FIRMWARE_VENDOR);
	
	// TODO: Fill in other efi properties if necessary
	
	// Set up the /efi/runtime-services table node similar to the way a child node of configuration-table
	// is set up.  That is, name and table properties
	Node *runtimeServicesNode = DT__AddChild(node, "runtime-services");
	
	if (archCpuType == CPU_TYPE_I386)
	{
		// The value of the table property is the 32-bit physical address for the RuntimeServices table.
		// Since the EFI system table already has a pointer to it, we simply use the address of that pointer
		// for the pointer to the property data.  Warning.. DT finalization calls free on that but we're not
		// the only thing to use a non-malloc'd pointer for something in the DT
		
		DT__AddProperty(runtimeServicesNode, "table", sizeof(uint64_t), &gST32->RuntimeServices);
	}
	else
	{
		DT__AddProperty(runtimeServicesNode, "table", sizeof(uint64_t), &gST64->RuntimeServices);
	}

	// Set up the /efi/configuration-table node which will eventually have several child nodes for
	// all of the configuration tables needed by various kernel extensions.
	gEfiConfigurationTableNode = DT__AddChild(node, "configuration-table");
	
	// Now fill in the /efi/platform Node
	Node *efiPlatformNode = DT__AddChild(node, "platform");
	
	// NOTE WELL: If you do add FSB Frequency detection, make sure to store
	// the value in the fsbFrequency global and not an malloc'd pointer
	// because the DT_AddProperty function does not copy its args.
	
	if (Platform.CPU.FSBFrequency != 0)
		DT__AddProperty(efiPlatformNode, FSB_Frequency_prop, sizeof(uint64_t), &Platform.CPU.FSBFrequency);
	
	// Export TSC and CPU frequencies for use by the kernel or KEXTs
	if (Platform.CPU.TSCFrequency != 0)
		DT__AddProperty(efiPlatformNode, TSC_Frequency_prop, sizeof(uint64_t), &Platform.CPU.TSCFrequency);
	
	if (Platform.CPU.CPUFrequency != 0)
		DT__AddProperty(efiPlatformNode, CPU_Frequency_prop, sizeof(uint64_t), &Platform.CPU.CPUFrequency);
	
	// Export system-id. Can be disabled with SystemId=No in com.apple.Boot.plist //Slice - nonsense
	if ((ret=getSystemID())){
		DT__AddProperty(efiPlatformNode, SYSTEM_ID_PROP, UUID_LEN, (EFI_UINT32*) ret);
	}

	 // Export SystemSerialNumber if present
	if ((ret16=getSmbiosChar16("SMserial", &len)))
		DT__AddProperty(efiPlatformNode, SYSTEM_SERIAL_PROP, len, ret16);
	
	// Export Model if present
	if ((ret16=getSmbiosChar16("SMproductname", &len)))
		DT__AddProperty(efiPlatformNode, MODEL_PROP, len, ret16);
	
	//Slice create /options node
	bool UseNVRAM = FALSE;
	bool ClearNVRAM = FALSE;
	char buff[20];
	int cnt;
	ClearNVRAM = getValueForKey(kClearNVRAM, &buff, &cnt, &bootInfo->bootConfig);

    getBoolForKey(kUseNVRAM, &UseNVRAM, &bootInfo->bootConfig);
	if (UseNVRAM) {
		
		Node* optionsNode = DT__FindNode("/options", true);
		DT__AddProperty(optionsNode, PLATFORM_UUID, UUID_LEN, (EFI_UINT32*) ret);		
		
		// this information can be obtained from DMI Type 0
		SMBByte* p = (SMBByte*)FindFirstDmiTableOfType(0, 0x18);
		FirmwareFeatures = malloc(sizeof(SMBByte));
		//TODO - the bufferFF must be composed from bits p[0x12] and [0x13]. Datasheet needed
		*FirmwareFeatures = ((p[19] >> 1) & 1)  //USB Legacy is supported
		| ((p[18] >> 14) & 2) //Boot from CD is supported
		| 0x14; //default for bless (GUID partition)
		
		sprintf(bootName, "%s:FirmwareFeatures", kBL_APPLE_VENDOR_NVRAM_GUID);
		ffName = malloc(sizeof(bootName)+1);
		strcpy(ffName, bootName);
		DT__AddProperty(optionsNode, ffName, sizeof(uint32_t), (char *)FirmwareFeatures); //legacy support
		
		sprintf(bootName, "%s:FirmwareFeaturesMask", kBL_APPLE_VENDOR_NVRAM_GUID);
		ffmName = malloc(sizeof(bootName)+1);
		strcpy(ffmName, bootName);	
		DT__AddProperty(optionsNode, ffmName, sizeof(uint32_t), (EFI_UINT32*)&FIRMWARE_FEATURE_MASK); 	
		
		//TODO - check, validate and fill by bvr structure.
		//here I am not sure what is BootOrder: node or property?
		//Node* bootNode = DT__AddChild(optionsNode, kBL_GLOBAL_NVRAM_GUID ":BootOrder");
		sprintf(bootName, "%s:BootOrder", kBL_GLOBAL_NVRAM_GUID);
		boName = malloc(sizeof(bootName)+1);
		strcpy(boName, bootName);		
		DT__AddProperty(optionsNode, boName, sizeof(uint32_t), (EFI_UINT32*)&STATIC_ZERO);	
		
		sprintf(bootName, "%s:Boot%04hx", kBL_GLOBAL_NVRAM_GUID, bootOptionNumber);
		bnName = malloc(sizeof(bootName)+1);
		strcpy(bnName, bootName);			
		DT__AddProperty(optionsNode, bnName, sizeof(uint32_t), (EFI_UINT32*)&STATIC_ZERO); 
		
		
		//can we add here boot-properties?
		//	optionsNode = DT__FindNode("chosen", true);
#if NOTYET
		int lbC = 0;
		while(((char*)&bootInfo->bootConfig)[lbC++]);
		if (lbC > sizeof(bootInfo->bootConfig)) lbC = sizeof(bootInfo->bootConfig);
		DT__AddProperty(optionsNode, "boot-args", lbC, (char*)&bootInfo->bootConfig);	
#endif
		//TODO - BootCamp emulation?
		/*
		 romNode = DT__FindNode("/rom", true);
		 DT__AddProperty(romNode, "fv-main-address"...  //provided by AppleSMBIOS
		 DT__AddProperty(romNode, "fv-main-size"...
		 "IOEFIDevicePathType" -> "MediaFirmwareVolumeFilePath"
		 "Guid" -> "2B0585EB-D8B8-49A9-8B8C-E21B01AEF2B7"
		 
		 */
		//end Slice
	}	
	
	// Fill /efi/device-properties node.
	setupDeviceProperties(node);
}

/*
 * Load the smbios.plist override config file if any
 */

static void setupSmbiosConfigFile(const char *filename)
{
	char		dirSpecSMBIOS[128] = "";
	const char *override_pathname = NULL;
	int			len = 0, err = 0;
	extern void scan_mem();
	
	// Take in account user overriding
	if (getValueForKey(kSMBIOSKey, &override_pathname, &len, &bootInfo->bootConfig) && len > 0)
	{
		// Specify a path to a file, e.g. SMBIOS=/Extra/macProXY.plist
		sprintf(dirSpecSMBIOS, override_pathname);
		err = loadConfigFile(dirSpecSMBIOS, &bootInfo->smbiosConfig);
	}
	else
	{
		// Check selected volume's Extra.
		sprintf(dirSpecSMBIOS, "/Extra/%s", filename);
		if (err = loadConfigFile(dirSpecSMBIOS, &bootInfo->smbiosConfig))
		{
			// Check booter volume/rdbt Extra.
			sprintf(dirSpecSMBIOS, "bt(0,0)/Extra/%s", filename);
			err = loadConfigFile(dirSpecSMBIOS, &bootInfo->smbiosConfig);
		}
	}

	if (err)
	{
		verbose("No SMBIOS replacement found.\n");
	}

	// get a chance to scan mem dynamically if user asks for it while having the config options loaded as well,
	// as opposed to when it was in scan_platform(); also load the orig. smbios so that we can access dmi info without
	// patching the smbios yet
	getSmbios(SMBIOS_ORIGINAL);
	scan_mem(); 
	smbios_p = (EFI_PTR32)getSmbios(SMBIOS_PATCHED);	// process smbios asap
//Slice
	bool useDMIinfoCPU = true;
    getBoolForKey(kUseCPUDMI, &useDMIinfoCPU, &bootInfo->bootConfig);
	if (useDMIinfoCPU) {
		scan_cpu_DMI(); //&Platform);
	}
	// PM_Model
//	if ((Platform.CPU.Features & CPU_FEATURE_MOBILE)) {
	if ((Platform.CPU.Mobile)) {
		Platform.Type = 2;
	} else {
		Platform.Type = 1;
	}
	
	//Slice - overclock CPU patch
	struct DMIProcessorInformation* p = (struct DMIProcessorInformation*) FindFirstDmiTableOfType(4, 0x28);
	
	if (useDMIinfoCPU && ((p->currentClock) && 
	//	(p->currentClock > p->maximumClock) &&
	//	(p->maximumClock != 3800) && (p->maximumClock != 4000) &&				  
		(p->currentClock < 10000)))
	{
		verbose("Overclocked CPU from %dMHz to %dMHz\n", p->maximumClock, p->currentClock);
		p->maximumClock = p->currentClock;
	}
}

/*
 * Installs all the needed configuration table entries
 */

static void setupEfiConfigurationTable()
{
//	smbios_p = (EFI_PTR32)getSmbios(SMBIOS_PATCHED);
	addConfigurationTable(&gEfiSmbiosTableGuid, &smbios_p, NULL);
	
	// Setup ACPI with DSDT overrides (mackerintel's patch)
	setupAcpi();
	
	// We've obviously changed the count.. so fix up the CRC32
	if (archCpuType == CPU_TYPE_I386)
	{
		gST32->Hdr.CRC32 = 0;
		gST32->Hdr.CRC32 = crc32(0L, gST32, gST32->Hdr.HeaderSize);
	}
	else
	{
		gST64->Hdr.CRC32 = 0;
		gST64->Hdr.CRC32 = crc32(0L, gST64, gST64->Hdr.HeaderSize);
	}
}

/*
 * Entrypoint from boot.c
 */

void setupFakeEfi(void)
{
	// Generate efi device strings 
	setup_pci_devs(root_pci_dev);

	// load smbios.plist file if any
	setupSmbiosConfigFile("SMBIOS.plist");
	
	// Initialize the base table
	if (archCpuType == CPU_TYPE_I386)
	{
		setupEfiTables32();
	}
	else
	{
		setupEfiTables64();
	}
	
	// Initialize the device tree
	setupEfiDeviceTree();
	
	// Add configuration table entries to both the services table and the device tree
	setupEfiConfigurationTable();
	struct DMIProcessorInformation* cpuInfo;
	struct DMIHeader * dmihdr;
	//Slice - Debug SMBIOS
	for (dmihdr = FindFirstDmiTableOfType(4, 30); dmihdr; dmihdr = FindNextDmiTableOfType(4, 30)) 
	{
		cpuInfo = (struct DMIProcessorInformation*)dmihdr;
		if (cpuInfo->processorType != 3) { // CPU
			continue;
		}
		//TODO validate
		msglog("Patched platform CPU Info:\n FSB=%d\n MaxSpeed=%d\n CurrentSpeed=%d\n", Platform.CPU.FSBFrequency/MEGA, Platform.CPU.TSCFrequency/MEGA, Platform.CPU.CPUFrequency/MEGA);
		
		msglog("Patched SMBIOS CPU Info:\n FSB=%d\n MaxSpeed=%d\n CurrentSpeed=%d\n", cpuInfo->externalClock, cpuInfo->maximumClock, cpuInfo->currentClock);
		msglog("\n Family=%x\n Socket=%x\n Cores=%d Enabled=%d Threads=%d\n", cpuInfo->processorFamily, cpuInfo->processorUpgrade, cpuInfo->coreCount, cpuInfo->coreEnabled, cpuInfo->Threads);
	}		
}

