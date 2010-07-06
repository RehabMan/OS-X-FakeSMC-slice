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
	
	// Hardware Detection Order
	OSDictionary* dictionary = OSDynamicCast(OSDictionary, getProperty("Detection Order"));
	
	if (dictionary)
	{
		OSCollectionIterator* i = OSCollectionIterator::withCollection(dictionary);
		
		while (OSSymbol* symbol = (OSSymbol*)i->getNextObject())
		{
			OSBoolean* enabled = OSDynamicCast(OSBoolean, dictionary->getObject(symbol));

			if (enabled && enabled->getValue())
			{
				if (symbol->isEqualTo("Fintek"))
				{
					superio = new Fintek();
					InfoLog("Probing Fintek");
				}
				else if (symbol->isEqualTo("Winbond"))
				{
					superio = new Winbond();
					InfoLog("Probing Winbond");
				}
				else if (symbol->isEqualTo("ITE"))
				{
					superio = new ITE();
					InfoLog("Probing ITE");
				}
				else if (symbol->isEqualTo("SMSC"))
				{
					superio = new SMSC();
					InfoLog("Probing SMSC");
				}
				
				if (superio)
				{
					if (superio->Probe()) 
					{
						InfoLog("Detected %s on 0x%x", superio->GetModelName(), superio->GetAddress());
						
						superio->LoadConfiguration(this);
						
						if (!m_WorkLoop)
							m_WorkLoop = getWorkLoop();
						
						if (!m_TimerEventSource)
							m_TimerEventSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &FakeSMCSuperIOMonitor::controllerTimerEvent));
						
						if (m_WorkLoop && m_TimerEventSource)
							m_WorkLoop->addEventSource(m_TimerEventSource);
						
						return this;
					}
					else 
					{
						delete superio;
					}
				}
			}
		}
		
		i->release();
	}
	
	InfoLog("No supported Super I/O chip has been detected!");
	
	return 0;
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