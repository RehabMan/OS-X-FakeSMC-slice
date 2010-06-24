/*
 *  SMSC.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 22/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _SMSC_H
#define _SMSC_H

#include "SuperIO.h"

const UInt8 SMSC_PORTS_COUNT = 2;
const UInt16 SMSC_PORT[] = { 0x2e, 0x4e };

class SMSC : public SuperIO
{
public:	
	virtual void	Enter();
	virtual void	Exit();
	
	virtual bool	ProbeCurrentPort();
	
	virtual void	Init();
	virtual void	Finish();
};

#endif