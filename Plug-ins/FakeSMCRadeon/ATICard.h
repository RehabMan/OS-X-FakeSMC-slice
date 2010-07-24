/*
 *  ATICard.h
 *  FakeSMCRadeon
 *
 *  Created by Slice on 24.07.10.
 *  Copyright 2010 Applelife.ru. All rights reserved.
 *
 */

#ifndef _ATICARD_H 
#define _ATICARD_H

#include <IOKit/IOService.h>
#include "FakeSMCBinding.h"

class ATICard : public FakeSMCBinding
{
private:
	Binding*			m_Sensor;
	Binding*			m_Controller;
	
	void			FlushList(Binding* start);
protected:
	IOService*		m_Service;
	
}
	

#endif _ATICARD_H