/*
 *  HWSensors.h
 *  IntelThermalPlugin
 *
 *  Based on code by mercurysquad, superhai (C) 2008
 *  Based on code from Open Hardware Monitor project by Michael Möller (C) 2011
 *
 *  Created by mozo on 30/09/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

/*
 
 Version: MPL 1.1/GPL 2.0/LGPL 2.1
 
 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 
 http://www.mozilla.org/MPL/
 
 Software distributed under the License is distributed on an "AS IS" basis,
 WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 for the specific language governing rights and limitations under the License.
 
 The Original Code is the Open Hardware Monitor code.
 
 The Initial Developer of the Original Code is 
 Michael Möller <m.moeller@gmx.ch>.
 Portions created by the Initial Developer are Copyright (C) 2011
 the Initial Developer. All Rights Reserved.
 
 Contributor(s):
 
 Alternatively, the contents of this file may be used under the terms of
 either the GNU General Public License Version 2 or later (the "GPL"), or
 the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 in which case the provisions of the GPL or the LGPL are applicable instead
 of those above. If you wish to allow use of your version of this file only
 under the terms of either the GPL or the LGPL, and not to allow others to
 use your version of this file under the terms of the MPL, indicate your
 decision by deleting the provisions above and replace them with the notice
 and other provisions required by the GPL or the LGPL. If you do not delete
 the provisions above, a recipient may use your version of this file under
 the terms of any one of the MPL, the GPL or the LGPL.
 
 */

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

#include "FakeSMCPlugin.h"

#include "cpuid.h"

#define MSR_IA32_THERM_STS      0x019C
#define MSR_IA32_TEMP_TARGET	0x01A2

#define MaxCpuCount 8

extern "C" void mp_rendezvous_no_intrs(void (*action_func)(void *), void * arg);
extern "C" int cpu_number(void);

static UInt8 GlobalThermal[MaxCpuCount];
static UInt16 GlobalStatus[MaxCpuCount];


inline UInt8 get_index(char c)
{
	return c >= 'A' ? c - 55 : c - 48;
};

inline UInt32 get_cpu_number()
{
    // I found information that reading from 1-4 cores gives the same result as reading from 5-8 cores for 4-cores 8-threads CPU. Needs more investigation
    return cpu_number() % cpuid_info()->core_count;
}

inline void read_cpu_diode(void* cpu_index)
{
    UInt32 * cpn = (UInt32 *)cpu_index;
    
	*cpn = get_cpu_number();

	if(*cpn < MaxCpuCount) {
		UInt64 msr = rdmsr64(MSR_IA32_THERM_STS);
		if (msr & 0x80000000) 
            GlobalThermal[*cpn] = (msr >> 16) & 0x7F;
	}
};

inline void read_cpu_status(void* cpu_index)
{
    UInt32 * cpn = (UInt32 *)cpu_index;
    
	*cpn = get_cpu_number();
    
	if(*cpn < MaxCpuCount) {
		UInt64 msr = rdmsr64(MSR_IA32_PERF_STS);
		GlobalStatus[*cpn] = msr & 0xFFFF;
	}
};

class IntelThermal : public FakeSMCPlugin
{
    OSDeclareDefaultStructors(IntelThermal)    
private:
    UInt8               tjmax[MaxCpuCount];
	void                readTjmaxFromMSR();
    
public:
    virtual IOService*		probe(IOService *provider, SInt32 *score);
    virtual bool			start(IOService *provider);
    
	virtual IOReturn	callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 ); 
	
};