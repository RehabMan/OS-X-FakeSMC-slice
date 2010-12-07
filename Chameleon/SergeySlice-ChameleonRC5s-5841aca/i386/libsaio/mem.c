/*
 * Copyright 2010 AsereBLN. All rights reserved. <aserebln@googlemail.com>
 *
 * mem.c - obtain system memory information
 */

#include "libsaio.h"
#include "pci.h"
#include "platform.h"
#include "cpu.h"
#include "mem.h"
#include "smbios_patcher.h"

#ifndef DEBUG_MEM
#define DEBUG_MEM 1
#endif

#if DEBUG_MEM
#define DBG(x...)		msglog(x)
#else
#define DBG(x...)
#endif

#define DC(c) (c >= 0x20 && c < 0x7f ? (char) c : '.')
#define STEP 16

void dumpPhysAddr(const char * title, void * a, int len)
{
    int i,j;
    u_int8_t* ad = (u_int8_t*) a;
    char buffer[80];
    char str[16];

    if(ad==NULL) return;

    DBG("%s addr=0x%08x len=%04d\n",title ? title : "Dump of ", a, len);
    DBG("Ofs-00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F       ASCII\n");
    i = (len/STEP)*STEP;
    for (j=0; j < i; j+=STEP)
    {
        DBG("%02x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
               j, 
               ad[j], ad[j+1], ad[j+2], ad[j+3] , ad[j+4], ad[j+5], ad[j+6], ad[j+7],
               ad[j+8], ad[j+9], ad[j+10], ad[j+11] , ad[j+12], ad[j+13], ad[j+14], ad[j+15],
               DC(ad[j]), DC(ad[j+1]), DC(ad[j+2]), DC(ad[j+3]) , DC(ad[j+4]), DC(ad[j+5]), DC(ad[j+6]), DC(ad[j+7]),
               DC(ad[j+8]), DC(ad[j+9]), DC(ad[j+10]), DC(ad[j+11]) , DC(ad[j+12]), DC(ad[j+13]), DC(ad[j+14]), DC(ad[j+15])
               ); 
    }

    if (len%STEP==0) return;
    sprintf(buffer,"%02x:", i);
    for (j=0; j < STEP; j++)  {
        if (j<(len%STEP))
            sprintf(str, " %02x", ad[i+j]);
        else
            strcpy(str, "   " );  
        strncat(buffer, str, sizeof(buffer));
    }
    strncat(buffer,"  ", sizeof(buffer));
    for (j=0; j < (len%STEP); j++)  {
        sprintf(str, "%c", DC(ad[i+j]));  
        strncat(buffer, str, sizeof(buffer));
    }
    DBG("%s\n",buffer);
}

void dumpAllTablesOfType(int i)
{
    char title[32];
        struct DMIHeader * dmihdr;
        for(dmihdr = FindFirstDmiTableOfType(i, 4);
            dmihdr;
            dmihdr = FindNextDmiTableOfType(i, 4)) {
            sprintf(title,"Table (type %d) :" , i); 
            dumpPhysAddr(title, dmihdr, dmihdr->length+32);
        }
}

const char * getDMIString(struct DMIHeader * dmihdr, uint8_t strNum)
{
    const char * ret =NULL;
    const char * startAddr = (const char *) dmihdr;
    const char * limit = NULL;

    if (!dmihdr || dmihdr->length<4 || strNum==0) return NULL;
    startAddr += dmihdr->length;
    limit = startAddr + 256;
    for(; strNum; strNum--) {
        if ((*startAddr)==0 && *(startAddr+1)==0) break;
        if (*startAddr && strNum<=1) {
            ret = startAddr; // current str
            break;
        }
        while(*startAddr && startAddr<limit) startAddr++;
        if (startAddr==limit) break; // no terminator found
        else if((*startAddr==0) && *(startAddr+1)==0) break;
        else startAddr++;
    }

    return ret;
}

void scan_memory(void) //PlatformInfo_t *p)
{
    int i=0;
    struct DMIHeader * dmihdr = NULL;
    
    struct DMIMemoryModuleInfo* memInfo[MAX_RAM_SLOTS]; // 6
    struct DMIPhysicalMemoryArray* physMemArray; // 16
    struct DMIMemoryDevice* memDev[MAX_RAM_SLOTS]; //17

    /* We mainly don't use obsolete tables 5,6 because most of computers don't handle it anymore */
     Platform.DMI.MemoryModules = 0;
    /* Now lets peek info rom table 16,17 as for some bios, table 5 & 6 are not used */
    physMemArray = (struct DMIPhysicalMemoryArray*) FindFirstDmiTableOfType(16, 4);
    Platform.DMI.MaxMemorySlots = physMemArray ? physMemArray->numberOfMemoryDevices :  0;
 
    i = 0;
    for(dmihdr = FindFirstDmiTableOfType(17, 4);
        dmihdr;
        dmihdr = FindNextDmiTableOfType(17, 4) ) {
        memDev[i] = (struct DMIMemoryDevice*) dmihdr;
        if (memDev[i]->size !=0 ) Platform.DMI.MemoryModules++;
        if (memDev[i]->memorySpeed>0) Platform.RAM.DIMM[i].Frequency = memDev[i]->memorySpeed; // take it here for now but we'll check spd and dmi table 6 as well
        i++;
    }
    // for table 6, we only have a look at the current speed
    i = 0;
    for(dmihdr = FindFirstDmiTableOfType(6, 4);
        dmihdr;
        dmihdr = FindNextDmiTableOfType(6, 4) ) {
        memInfo[i] = (struct DMIMemoryModuleInfo*) dmihdr;
        if (memInfo[i]->currentSpeed > Platform.RAM.DIMM[i].Frequency) 
            Platform.RAM.DIMM[i].Frequency = memInfo[i]->currentSpeed; // favor real overclocked speed if any
        i++;
    }
#if DEBUG_MEM
    dumpAllTablesOfType(17);
    //getc();
#endif
}
//Slice
#define MEGA 1000000LL
void scan_cpu_DMI(void) //PlatformInfo_t *p)
{
//    int i=0;
    struct DMIHeader * dmihdr = NULL;    
    struct DMIProcessorInformation* cpuInfo; // Type 4
	
	for (dmihdr = FindFirstDmiTableOfType(4, 30); dmihdr; dmihdr = FindNextDmiTableOfType(4, 30)) 
	{
		cpuInfo = (struct DMIProcessorInformation*)dmihdr;
		if (cpuInfo->processorType != 3) { // CPU
			continue;
		}
		//TODO validate
#if 1 //NOTYET	
		msglog("Platform CPU Info:\n FSB=%d\n MaxSpeed=%d\n CurrentSpeed=%d\n", Platform.CPU.FSBFrequency/MEGA, Platform.CPU.TSCFrequency/MEGA, Platform.CPU.CPUFrequency/MEGA);

		if ((cpuInfo->externalClock) && (cpuInfo->externalClock < 400)) {  //<400MHz
			Platform.CPU.FSBFrequency = (cpuInfo->externalClock) * MEGA;
		}
		if ((cpuInfo->maximumClock) && (cpuInfo->maximumClock < 10000)) {  //<10GHz
			Platform.CPU.TSCFrequency = cpuInfo->maximumClock * MEGA;
		}
		if ((cpuInfo->currentClock) && (cpuInfo->currentClock < 10000)) {  //<10GHz
			Platform.CPU.CPUFrequency = cpuInfo->currentClock * MEGA;
		}
#endif
		msglog("DMI CPU Info:\n FSB=%d\n MaxSpeed=%d\n CurrentSpeed=%d\n", cpuInfo->externalClock, cpuInfo->maximumClock, cpuInfo->currentClock);
		msglog("DMI CPU Info 2:\n Family=%x\n Socket=%x\n Cores=%d Enabled=%d Threads=%d\n", cpuInfo->processorFamily, cpuInfo->processorUpgrade, cpuInfo->coreCount, cpuInfo->coreEnabled, cpuInfo->Threads);
#if 1 //NOTYET
		if ((cpuInfo->coreCount) && (cpuInfo->coreCount<Platform.CPU.NoCores)) {
			if (cpuInfo->coreEnabled < cpuInfo->coreCount) {
				cpuInfo->coreCount = cpuInfo->coreEnabled;
			}
			Platform.CPU.NoCores = cpuInfo->coreCount;
		}
		if ((cpuInfo->Threads) && (cpuInfo->Threads<Platform.CPU.NoThreads)) {		
			Platform.CPU.NoThreads = cpuInfo->Threads;
		}
#endif
		
		return;
	}

	return;
}
//Slice - check other DMI info
bool scanDMI(void)
{
	struct DMIHeader * dmihdr = NULL;    
    struct DMISystemEnclosure* encInfo; // Type 3

	for (dmihdr = FindFirstDmiTableOfType(3, 13); dmihdr; dmihdr = FindNextDmiTableOfType(3, 13)) 
	{
		encInfo = (struct DMISystemEnclosure*)dmihdr;
		msglog("DMI Chassis Info:\n Type=%x\n Boot-up State=%x\n Power Supply=%x Thermal State=%x\n", encInfo->type, encInfo->bootupState, encInfo->powerSupplyState, encInfo->thermalState);
		switch (encInfo->type) {
			case 1:
			case 2:
				return FALSE;
			case 3:
			case 4:
			case 6:
			case 7:
				Platform.CPU.Mobile = FALSE;
				break;
			case 8:
			case 9:
			case 0x0A:
			case 0x0B:				
			case 0x0E:
				Platform.CPU.Mobile = TRUE;
				break;
				
			default:
				break;
		}
		return TRUE;
	}
	return FALSE;	
}