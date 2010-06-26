/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 */

#include "ITESMCController.h"

#define super IOService
OSDefineMetaClassAndStructors(TheOnlyWorkingITESMCController, IOService)

bool TheOnlyWorkingITESMCController::init(OSDictionary *properties) {
	//DebugLog("Initializing...");	
    super::init(properties);	
	return true;
}

IOService* TheOnlyWorkingITESMCController::probe(IOService *provider, SInt32 *score) {
	//DebugLog("Probing...");	
	if (super::probe(provider, score) != this) 
		return 0;	
	superio = new ITE();			
	if(!superio->Probe())
		return false;		
	InfoLog("Found %s Super I/O chip", superio->GetModelName());
	superio->LoadConfiguration(this);	
	return this;
}

bool TheOnlyWorkingITESMCController::start(IOService * provider) {
	//DebugLog("Starting...");	
	if (!super::start(provider)) return false;	
	WorkLoop = IOWorkLoop::workLoop();
	TimerEventSource = IOTimerEventSource::timerEventSource(this,
															OSMemberFunctionCast(IOTimerEventSource::Action, this,
																				 &TheOnlyWorkingITESMCController::LoopTimerEvent));
	
	TimerEventSource->setTimeoutMS(100);
	return !((!superio)||(!WorkLoop)||(!TimerEventSource)||(kIOReturnSuccess != WorkLoop->addEventSource(TimerEventSource)));
}

void TheOnlyWorkingITESMCController::stop (IOService* provider) {
	//DebugLog("Stopping...");	
	if(superio)
		superio->Finish();
	super::stop(provider);
}

void TheOnlyWorkingITESMCController::free () {
	//DebugLog("Freeing...");	
	delete superio;	
	super::free ();
}

IOWorkLoop*	TheOnlyWorkingITESMCController::getWorkLoop()const {
	return WorkLoop;
}

IOReturn	TheOnlyWorkingITESMCController::LoopTimerEvent() {
	TimerEventSource->disable();
	TimerEventSource->release();
	if(superio)
		superio->Init();
	else 
		return kIOReturnNoDevice;
	return kIOReturnSuccess;
}