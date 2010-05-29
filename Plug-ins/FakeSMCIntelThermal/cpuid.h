/*
 *  cpuid.h
 *
 *  Created by mark on 7/6/09.
 *  Copyright 2009 markswell. All rights reserved.
 *
 */

#define _Bit(n)         (1ULL << n)
//#define _HBit(n)        (1ULL << ((n) + 32))

#define min(a,b)        ((a) < (b) ? (a) : (b))
#define quad(hi,lo)     (((uint64_t)(hi)) << 32 | (lo))
#define bit(n)          (1UL << (n))
#define bitmask(h,l)    ((bit(h) | (bit(h) - 1)) & ~(bit(l) - 1))
#define bitfield(x,h,l) (((x) & bitmask(h, l)) >> l)

static i386_cpu_info_t cpuid_cpu_info;

i386_cpu_info_t	*cpuid_info(void)
{
	return 	&cpuid_cpu_info;
}

static void cpuid_update_generic_info()
{
    uint32_t cpuid_reg[4];
    uint32_t max_extid;
    char     str[128];
    char*    p;
	i386_cpu_info_t* info_p = cpuid_info();
	
    /* Get vendor */
    do_cpuid(0, cpuid_reg);
    bcopy((char *)&cpuid_reg[ebx], &info_p->cpuid_vendor[0], 4); /* ug */
    bcopy((char *)&cpuid_reg[ecx], &info_p->cpuid_vendor[8], 4);
    bcopy((char *)&cpuid_reg[edx], &info_p->cpuid_vendor[4], 4);
    info_p->cpuid_vendor[12] = 0;
	
    /* Get extended CPUID results */
    do_cpuid(0x80000000, cpuid_reg);
    max_extid = cpuid_reg[eax];
	
    /* Check to see if we can get the brand string */
    if (max_extid >= 0x80000004) {
        /*
         * The brand string is up to 48 bytes and is guaranteed to be
         * NUL terminated.
         */
        do_cpuid(0x80000002, cpuid_reg);
        bcopy((char *)cpuid_reg, &str[0], 16);
        do_cpuid(0x80000003, cpuid_reg);
        bcopy((char *)cpuid_reg, &str[16], 16);
        do_cpuid(0x80000004, cpuid_reg);
        bcopy((char *)cpuid_reg, &str[32], 16);
        for (p = str; *p != '\0'; p++) {
            if (*p != ' ') break;
        }
        strncpy(info_p->cpuid_brand_string, p,
                sizeof(info_p->cpuid_brand_string));
		
        if (!strncmp(info_p->cpuid_brand_string, CPUID_STRING_UNKNOWN,
					 min(sizeof(info_p->cpuid_brand_string),
						 strlen(CPUID_STRING_UNKNOWN) + 1))) {
						 /*
						  * This string means we have a firmware-programmable brand string,
						  * and the firmware couldn't figure out what sort of CPU we have.
						  */
						 info_p->cpuid_brand_string[0] = '\0';
					 }
    }
    
    /* Get cache and addressing info */
    if (max_extid >= 0x80000006) {
        do_cpuid(0x80000006, cpuid_reg);
        info_p->cpuid_cache_linesize = bitfield(cpuid_reg[ecx], 7, 0);
        info_p->cpuid_cache_L2_associativity = bitfield(cpuid_reg[ecx], 15, 12);
        info_p->cpuid_cache_size = bitfield(cpuid_reg[ecx], 31, 16);
        do_cpuid(0x80000008, cpuid_reg);
        info_p->cpuid_address_bits_physical = bitfield(cpuid_reg[eax], 7, 0);
        info_p->cpuid_address_bits_virtual = bitfield(cpuid_reg[eax], 15, 8);
    }
	
    /* Get processor signature and decode */
    do_cpuid(1, cpuid_reg);
    info_p->cpuid_signature = cpuid_reg[eax];
    info_p->cpuid_stepping  = bitfield(cpuid_reg[eax],  3,  0);
    info_p->cpuid_model     = bitfield(cpuid_reg[eax],  7,  4);
    info_p->cpuid_family    = bitfield(cpuid_reg[eax], 11,  8);
    info_p->cpuid_type      = bitfield(cpuid_reg[eax], 13, 12);
    info_p->cpuid_extmodel  = bitfield(cpuid_reg[eax], 19, 16);
    info_p->cpuid_extfamily = bitfield(cpuid_reg[eax], 27, 20);
    info_p->cpuid_brand     = bitfield(cpuid_reg[ebx],  7,  0);
    info_p->cpuid_features  = quad(cpuid_reg[ecx], cpuid_reg[edx]);
	
    /* Fold extensions into family/model */
    if (info_p->cpuid_family == 0x0f) {
        info_p->cpuid_family += info_p->cpuid_extfamily;
    }
    if (info_p->cpuid_family == 0x0f || info_p->cpuid_family== 0x06) {
        info_p->cpuid_model += (info_p->cpuid_extmodel << 4);
    }
	
    if (info_p->cpuid_features & CPUID_FEATURE_HTT) {
        info_p->cpuid_logical_per_package = bitfield(cpuid_reg[ebx], 23, 16);
    } else {
        info_p->cpuid_logical_per_package = 1;
    }
	
    if (max_extid >= 0x80000001) {
        do_cpuid(0x80000001, cpuid_reg);
        info_p->cpuid_extfeatures = quad(cpuid_reg[ecx], cpuid_reg[edx]);
    }
	
    if (info_p->cpuid_extfeatures & CPUID_FEATURE_MONITOR) {
		
        do_cpuid(5, cpuid_reg);
        info_p->cpuid_mwait_linesize_min = cpuid_reg[eax];
        info_p->cpuid_mwait_linesize_max = cpuid_reg[ebx];
        info_p->cpuid_mwait_extensions   = cpuid_reg[ecx];
        info_p->cpuid_mwait_sub_Cstates  = cpuid_reg[edx];
		
        do_cpuid(6, cpuid_reg);
        info_p->cpuid_thermal_sensor = bitfield(cpuid_reg[eax], 0, 0);
        info_p->cpuid_thermal_dynamic_acceleration =
		bitfield(cpuid_reg[eax], 1, 1);
        info_p->cpuid_thermal_thresholds = bitfield(cpuid_reg[ebx], 3, 0);
        info_p->cpuid_thermal_ACNT_MCNT = bitfield(cpuid_reg[ecx], 0, 0);
		
        do_cpuid(0xa, cpuid_reg);
        info_p->cpuid_arch_perf_version = bitfield(cpuid_reg[eax], 7, 0);
        info_p->cpuid_arch_perf_number = bitfield(cpuid_reg[eax],15, 8);
        info_p->cpuid_arch_perf_width = bitfield(cpuid_reg[eax],23,16);
        info_p->cpuid_arch_perf_events_number = bitfield(cpuid_reg[eax],31,24);
        info_p->cpuid_arch_perf_events = cpuid_reg[ebx];
        info_p->cpuid_arch_perf_fixed_number = bitfield(cpuid_reg[edx], 4, 0);
        info_p->cpuid_arch_perf_fixed_width = bitfield(cpuid_reg[edx],12, 5);
		
    }
}
uint32_t cpuid_count_cores()
{
	uint32_t cpuid_reg[4];
	
	do_cpuid(1, cpuid_reg);
	
	return bitfield(cpuid_reg[1], 23, 16);
}

