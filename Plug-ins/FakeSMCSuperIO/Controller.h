/*
 *  Controller.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 27/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

class Controller
{
public:
	Controller*	Next;
	
	virtual void TimerEvent() {};
};

#endif