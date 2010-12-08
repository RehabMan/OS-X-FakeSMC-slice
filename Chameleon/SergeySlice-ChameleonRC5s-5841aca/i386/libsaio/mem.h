/*
 * Copyright 2010 AsereBLN. All rights reserved. <aserebln@googlemail.com>
 *
 * mem.h
 */

#ifndef __LIBSAIO_MEM_H
#define __LIBSAIO_MEM_H

#include "platform.h"
#define MEGA 1000000LL

extern void scan_memory(void); //PlatformInfo_t *);
extern void scan_cpu_DMI(void); //PlatformInfo_t *); //Slice
extern bool scanDMI(void);


#endif	/* __LIBSAIO_MEM_H */
