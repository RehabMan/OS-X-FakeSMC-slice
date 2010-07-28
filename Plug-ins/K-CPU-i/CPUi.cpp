#include <CPUi.h>
#include <IOKit/IODeviceTreeSupport.h>

OSDefineMetaClassAndStructors(CPUi, IOService);

IOReturn FrequencySensor::OnKeyRead(const char* key, char* data)
{
	int i=key[3]-'0';
	data[0]=Frequency[i]>>8;
	data[1]=Frequency[i]&0xff;
	return kIOReturnSuccess;
}

IOReturn TemperatureSensor::OnKeyRead(const char* key, char* data)
{
	int i=key[2]-'0';
	if (i > MaxCpuCount)
		return kIOReturnNoDevice;
	
	UInt32 magic = 0;		
	mp_rendezvous_no_intrs(IntelThermal, &magic);
	
	data[0]=CpuTjmax-GlobalThermal[i];
	data[1]=0;
	return kIOReturnSuccess;
}


void CPUi::Activate(void)
{
	if(Active)
		return;
	
	
	
	Active = true;
	
	loopTimerEvent();
	
	InfoLog("Monitoring started");
}

void CPUi::Deactivate(void)
{
	if(!Active)
		return;
	
	if (TimerEventSource)
		TimerEventSource->cancelTimeout();
	
	Active = false;
	
	InfoLog("Monitoring stopped");
}

IOReturn CPUi::loopTimerEvent(void)
{
	//Please, don't remove this timer! If frequency is read in OnKeyRead function, then the CPU is loaded by smcK-Stat-i and
	//goes to a higher P-State in this moment, displays high frequency and switches back to low frequency.
	UInt32 magic = 0;	
	
	TimerEventSource->setTimeoutMS(1000);
	
	if(LoopLock)
		return kIOReturnTimeout;
	
	LoopLock = true;
	
	// State Readout
	mp_rendezvous_no_intrs(IntelState, &magic);
	
			
	for (UInt32 i = 0; i < CpuCount; i++) 
	{
		Frequency[i] = IntelGetFrequency(GlobalState[i].FID, BusClock);
		Voltage[i] = IntelGetVoltage(GlobalState[i].VID);
	}
	LoopLock = false;
	
	return kIOReturnSuccess;
}


IOService * CPUi::probe(IOService * provider, SInt32 * score)
{
	DebugLog("probing...");
	
	if (IOService::probe(provider, score) != this) return 0;
	
	
	i386_cpu_info_t* cpuinfo=cpuid_info();
	if (strcmp(cpuinfo->cpuid_vendor, CPUID_VID_INTEL) != 0)
	{
		WarningLog("No Intel processor found, kext will not load");
		return NULL;
	}
	
	uint64_t features = cpuinfo->cpuid_features;
	
	if(!(features & CPUID_FEATURE_MSR))
	{
		WarningLog("Processor does not support Model Specific Registers, kext will not load");
		return 0;
	}
	
	if(!(cpuinfo->cpuid_features & CPUID_FEATURE_EST))
	{
		WarningLog("Processor does not support Enhanced SpeedStep, kext will not load");
		return NULL;
	}
	
	CpuCount = 0;
	
	OSDictionary * iocpu = serviceMatching("IOCPU");
	
	if (iocpu != NULL) 
	{
		OSIterator * iterator = getMatchingServices(iocpu);
		
		if (iterator != NULL) 
		{
			while (CpuArray[CpuCount] = OSDynamicCast(IOCPU, iterator->getNextObject()))
			{
				DebugLog("Got I/O Kit CPU %d (%d) named %s", CpuCount, CpuArray[CpuCount]->getCPUNumber(), CpuArray[CpuCount]->getCPUName()->getCStringNoCopy());
				
				if (CpuCount++ > MaxCpuCount)
					break;
			}
		}
	}
	
	if(CpuCount == 0)
	{
		WarningLog("IOKit CPUs not found, kext will not load");
		return 0;
	}

	BusClock = gPEClockFrequencyInfo.bus_frequency_max_hz >> 2;
	
	IORegistryEntry * entry = fromPath("/efi/platform", gIODTPlane);
	
	if (entry)
	{
		OSObject * object = entry->getProperty("FSBFrequency");
		
		if (object && (OSTypeIDInst(object) == OSTypeID(OSData))) 
		{
			OSData * data = OSDynamicCast(OSData, object);
			
			if (data) 
			{
				BusClock = * (UInt32 *) data->getBytesNoCopy();
				gPEClockFrequencyInfo.bus_frequency_max_hz = BusClock << 2;
			}
		}
	}	
	
	BusClock = BusClock / 1000000;
	FSBClock = gPEClockFrequencyInfo.bus_frequency_max_hz / 1000000;
	
	CpuSignature = cpuinfo->cpuid_signature;
	CpuCoreTech = Unknown;
	CpuNonIntegerBusRatio = (rdmsr64(MSR_IA32_PERF_STS) & (1ULL << 46));
	
	// Netburst
	switch (CpuSignature & 0x000FF0) 
	{
		case 0x000F10:
		case 0x000F20:
			CpuCoreTech = IntelNetburstOld;
			break;
		case 0x000F30:
		case 0x000F40:
		case 0x000F60:
			CpuCoreTech = IntelNetburstNew;
			break;
	}
	
	// Core/P-M
	switch (CpuSignature & 0x0006F0) 
	{
		case 0x000690:
		case 0x0006D0:
			CpuCoreTech = IntelPentiumM;
			break;
		case 0x0006E0:
		case 0x0006F0:
			CpuCoreTech = IntelCore;
			break;
	}
	
	// Core/Core45/i7
	switch (CpuSignature & 0x0106F0) 
	{
		case 0x010660:
			CpuCoreTech = IntelCore;
			break;
		case 0x010670:
		case 0x0106D0:
			CpuCoreTech = IntelCore45;
			break;
		case 0x0106C0:
			CpuCoreTech = IntelAtom;
			break;
		case 0x0106A0:
			CpuCoreTech = IntelCoreI7;
			break;
	}

	if (CpuCoreTech == Unknown) 
		WarningLog("CPU Core technology unknown");

	bool CpuTjmax15 = false;
	
	// Check CPU is mobile
	switch (CpuCoreTech) 
	{
		case IntelPentiumM:
			CpuMobile = true;
			break;
		case IntelNetburstOld:
			CpuMobile = (rdmsr64(MSR_P4_EBC_FREQUENCY_ID) & (1 << 21));
			break;
		case IntelNetburstNew:
			CpuMobile = (rdmsr64(MSR_P4_EBC_FREQUENCY_ID) & (1 << 21));
			break;
		case IntelCoreI7:
		case IntelCore:
		case IntelCore45:
		case IntelAtom:
		{
			CpuMobile = (rdmsr64(MSR_IA32_PLATFORM_ID) & (1 << 28));
			
			if (rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 27)) 
			{
				wrmsr64(MSR_IA32_EXT_CONFIG, (rdmsr64(MSR_IA32_EXT_CONFIG) | (1 << 28))); IOSleep(1);
				CpuDynamicFSB = rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 28);
			}
			
			CpuTjmax15 = (rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 30));
			
			break;
		}
	}

	CpuTjmax = 100;
	
	// Find a TjMAX value
	switch (CpuCoreTech) 
	{
		case IntelCore45:
		{
			if (CpuMobile) 
			{
				CpuTjmax += 5;
				
				if (CpuTjmax15)
					CpuTjmax -= 15;
			}
			
			break;
		}
		case IntelAtom:
		{
			if (CpuMobile) 
				CpuTjmax -= 10;
			
			break;
		}
		case IntelCore:
		{
			if ((CpuMobile) && (CpuTjmax15)) 
				CpuTjmax -= 15;
			
			break;
		}
		case IntelCoreI7:
		{
			CpuTjmax = ((rdmsr64(MSR_IA32_TEMPERATURE_TARGET) >> 16) & 0xFF);
			break;
		}
	}	

	DebugLog("Tjmax %u, Mobile CPU %u", CpuTjmax, CpuMobile);
	
	// Retrieving P-States
	
	PStateInitial.Control = rdmsr64(MSR_IA32_PERF_STS) & 0xFFFF;
	
	PStateMaximum.Control = ((rdmsr64(MSR_IA32_PERF_STS) >> 32) & 0x1F3F) | (0x4000 * CpuNonIntegerBusRatio);
	PStateMaximum.CID = ((PStateMaximum.FID & 0x1F) << 1) | CpuNonIntegerBusRatio;
	
	PStateMinimum.FID = ((rdmsr64(MSR_IA32_PERF_STS) >> 24) & 0x1F) | (0x80 * CpuDynamicFSB);
	PStateMinimum.VID = ((rdmsr64(MSR_IA32_PERF_STS) >> 48) & 0x3F);
	
	if (PStateMinimum.FID == 0) 
	{
		// Probe for lowest fid
		for (UInt8 i = PStateMaximum.FID; i >= 0x6; i--) 
		{
			wrmsr64(MSR_IA32_PERF_CTL, (rdmsr64(MSR_IA32_PERF_CTL) & 0xFFFFFFFFFFFF0000ULL) | (i << 8) | PStateMinimum.VID);
			IntelWaitForSts();
			PStateMinimum.FID = (rdmsr64(MSR_IA32_PERF_STS) >> 8) & 0x1F; IOSleep(1);
		}
		
		wrmsr64(MSR_IA32_PERF_CTL, (rdmsr64(MSR_IA32_PERF_CTL) & 0xFFFFFFFFFFFF0000ULL) | (PStateMaximum.FID << 8) | PStateMaximum.VID);
		IntelWaitForSts();
	}
	
	if (PStateMinimum.VID == PStateMaximum.VID) 
	{	// Probe for lowest vid
		for (UInt8 i = PStateMaximum.VID; i > 0xA; i--) 
		{
			wrmsr64(MSR_IA32_PERF_CTL, (rdmsr64(MSR_IA32_PERF_CTL) & 0xFFFFFFFFFFFF0000ULL) | (PStateMinimum.FID << 8) | i);
			IntelWaitForSts();
			PStateMinimum.VID = rdmsr64(MSR_IA32_PERF_STS) & 0x3F; IOSleep(1);
		}
		
		wrmsr64(MSR_IA32_PERF_CTL, (rdmsr64(MSR_IA32_PERF_CTL) & 0xFFFFFFFFFFFF0000ULL) | (PStateMaximum.FID << 8) | PStateMaximum.VID);
		IntelWaitForSts();
	}
	
	PStateMinimum.CID = ((PStateMinimum.FID & 0x1F) << 1) >> CpuDynamicFSB;
	
	// Sanity check
	if (PStateMaximum.CID < PStateMinimum.CID) 
	{
		WarningLog("Insane FID values");
		
		CpuClock = gPEClockFrequencyInfo.cpu_frequency_max_hz / 1000000;
		PStatesCount = 1;
	}
	else
	{
		// Finalize P-States
		// Find how many P-States machine supports
		PStatesCount = PStateMaximum.CID - PStateMinimum.CID + 1;
		
		if (PStatesCount > MaxPStateCount) 
			PStatesCount = MaxPStateCount;

		UInt8 vidstep;
		UInt8 i = 0, invalid = 0;

		vidstep = ((PStateMaximum.VID << 2) - (PStateMinimum.VID << 2)) / (PStatesCount - 1);

		for (UInt8 u = 0; u < PStatesCount; u++) 
		{
			i = u - invalid;
			
			PStates[i].CID = PStateMaximum.CID - u;
			PStates[i].FID = (PStates[i].CID >> 1);
			
			if (PStates[i].FID < 0x6) 
				if (CpuDynamicFSB) 
					PStates[i].FID = (PStates[i].FID << 1) | 0x80;
			else if (CpuNonIntegerBusRatio) 
				PStates[i].FID = PStates[i].FID | (0x40 * (PStates[i].CID & 0x1));
						
			if (i && PStates[i].FID == PStates[i-1].FID)
				invalid++;
			
			PStates[i].VID = ((PStateMaximum.VID << 2) - (vidstep * u)) >> 2;
		}

		PStatesCount -= invalid;
		
		CpuClock = IntelGetFrequency(PStates[0].FID, BusClock);
	}
	
	return this;
}

bool CPUi::start(IOService * provider)
{
	DebugLog("starting...");
	
	if (!IOService::start(provider)) return false;
	
	InfoLog("(C) 2009 Mojodojo, All Rights Reserved");
	InfoLog("(C) 2009 Code based on Superhai's VoodooPower and Mercurysquad's IntelEnhancedSpeedStep, All Rights Reserved");
	
	// Setup loop event timer
	
	if (!(WorkLoop = getWorkLoop())) 
		return false;
	
	if (!(TimerEventSource = IOTimerEventSource::timerEventSource( this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &CPUi::loopTimerEvent)))) 
		return false;
	
	if (kIOReturnSuccess != WorkLoop->addEventSource(TimerEventSource))
	{
		return false;
	}
	
	//this->registerService(0);
	Activate();
	for (UInt32 i = 0; i < CpuCount; i++) 
	{
		char key[5];
		snprintf(key, 5, "FRC%d", (int)i);
		FreqBinding[i]=new FrequencySensor(key, "freq", 2);
		snprintf(key, 5, "TC%dD", (int)i);
		TempBinding[i]=new TemperatureSensor(key, "sp78", 2);
	}
	
	return true;
}

void CPUi::stop(IOService * provider)
{	
	DebugLog("stopping...");
	
	Deactivate();
	
	for (int i=0; i<CpuCount; i++) {
		if (FreqBinding[i])
			delete FreqBinding[i];
		if (TempBinding[i])
			delete TempBinding[i];
	}
	
	IOService::stop(provider);
}

void CPUi::free(void)
{
	DebugLog("unloading...");
		
	IOService::free();
}

