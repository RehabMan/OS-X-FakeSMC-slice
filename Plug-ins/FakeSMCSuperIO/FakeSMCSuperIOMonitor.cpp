/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 */

#include "FakeSMCSuperIOMonitor.h"

#define super IOService
OSDefineMetaClassAndStructors(FakeSMCSuperIOMonitor, IOService)

IOReturn FakeSMCSuperIOMonitor::controllerTimerEvent(void)
{
	m_TimerEventSource->setTimeoutMS(3000);
	
	if (superio)
	{
		superio->ControllersTimerEvent();
		return kIOReturnSuccess;
	}
	
	return kIOReturnInvalid;
}

bool FakeSMCSuperIOMonitor::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
    super::init(properties);
	
	return true;
}

IOService* FakeSMCSuperIOMonitor::probe(IOService *provider, SInt32 *score)
{
	DebugLog("Probing...");
	
	if (super::probe(provider, score) != this) return 0;
	
	superio = new Fintek();
	
	InfoLog("Probing Fintek");
			
	if(!superio->Probe())
	{
		delete superio;
		
		superio = new Winbond();
		
		InfoLog("Probing Winbond");
				
		if(!superio->Probe())
		{
			delete superio;
						
			superio = new SMSC();
			
			InfoLog("Probing SMSC");
			
			if(!superio->Probe())
			{
				delete superio;
			
				superio = new ITE();
				
				InfoLog("Probing ITE");
								
				if(!superio->Probe())
				{
					delete superio;
					
					InfoLog("No supported Super I/O chip has been found!");
					return 0;
				}
			}
		}
	}
	
	InfoLog("Found %s Super I/O chip on 0x%x", superio->GetModelName(), superio->GetAddress());

	superio->LoadConfiguration(this);
	
	if (!m_WorkLoop)
		m_WorkLoop = getWorkLoop();
	
	if (!m_TimerEventSource)
		m_TimerEventSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &FakeSMCSuperIOMonitor::controllerTimerEvent));
	
	if (m_WorkLoop && m_TimerEventSource)
		m_WorkLoop->addEventSource(m_TimerEventSource);
	
	return this;
}

bool FakeSMCSuperIOMonitor::start(IOService * provider)
{
	DebugLog("Starting...");
	
	if (!super::start(provider)) return false;
	
	if(superio)
	{
		superio->Start();
		
		if (superio->GetControllers())
		{
			if (m_WorkLoop && m_TimerEventSource)
			{
				controllerTimerEvent();
			}
			else 
			{
				WarningLog("Controllers workloop doesn't activated!");
			}
		}
	}
	else 
	{
		return false;
	}

	return true;
}

void FakeSMCSuperIOMonitor::stop (IOService* provider)
{
	DebugLog("Stoping...");
	
	if (m_TimerEventSource)
		m_TimerEventSource->cancelTimeout();
	
	if(superio)
		superio->Stop();

	super::stop(provider);
}

void FakeSMCSuperIOMonitor::free ()
{
	DebugLog("Freeing...");
	
	if (superio)
		delete superio;
	
	super::free ();
}