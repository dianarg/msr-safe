#ifndef MSR_SUPPLEMENTAL_H
#define MSR_SUPPLEMENTAL_H

/* TLCC2 machines are based on Sandy Bridge Server processors, family 06 model 2D.*/


#define SMSR_NOWRITE  (0x0)
#define SMSR_NOREAD   (0x0)
#define SMSR_READALL  (0xFFFFFFFF)
#define SMSR_WRITEALL (0xFFFFFFFF)

#ifdef _USE_ARCH_062D

/* TODO:  uncore? */

/* 
 * I assume that no MSR on this list contains sensitive information.  Reads
 * will return the unmodified contents of the entire MSR.
 *
 * There are six cases for writes, several of which may exist within a single
 * MSR:
 *
 * a) Reserved					mask=0
 * b) Read-Only					mask=0
 * c) Read/Write, but read-only per policy	mask=0
 * d) Clear-Only, but read-only per policy	mask=0
 * e) Read/Write				mask=1
 * f) Clear-Only				mask=1
 *
 * Intel provides the following guidance:
 * "When loading a register, always load the reserved bits with the values 
 * indicated in the documentation, if any, or reload them with values previously 
 * read from the same register." (Section 1.3.2)
 *
 * Based on this, all writes will execute a read-modify-write sequence as
 * follows:
 *
 * 	existingval	<- rdmsr() & ~writemask
 * 	maskedval	<- userval & writemask
 * 			   wrmsr( maskedval | temp )
 *
 *
 * References are to the September 2013 edition of the Intel documentation.
 *
 * Values are for processor family 06 model 2D, "Intel Xeon Processor E5 Family
 * based on the Intel microarchitecture code name Sandy Bridge" (Table 35-1).
 *
 * Relevant tables are:
 * 	Page 35-102
 * 	Section 35.8 MSRs in Intel Processor Family Based on Intel Microarchitecture Sandy Bridge
 * 	Table 35-12 MSRs Suppored by Intel Processors based on Intel microarchitecture code name Sandy Bridge
 *
 * 	Page 35-121
 * 	Section 35.8.2 MSRs in Intel Xeon Processor E5 Family 
 * 	Table 35-14 Selected MSRs Supported by Intel Xeon Processors E5 Family
 *
 * 	Page 35-2
 * 	Section 35.1
 * 	Table 35-2 IA-32 Architectural MSRs
 *
 * Architectural information should only be used when indicated by tables 35-12 and 35-14.
 *
 *	IA32_TIME_STAMP_COUNTER		See section 17.13.  Restrict to RO.
 *	 Thread		RW (RO)		
 *	 0x00 0x00
 *
 *	IA32_PLATFORM_ID		Bits 52:50 are of interest.  
 *	 Package	RO		
 *	 0x00 0x00
 *	
 *	PMCn				
 *	 0-3 Thread	RW		No restricted bits.
 *	 4-7 Core	RW		
 * 	 0xFFFFFFFF 0xFFFFFFFF
 *
 * 	MPERF/APERF			Restrict to RO.  See Section 14.2.		
 * 	 Thread		RW (RO)
 *
 *	PERFEVTSELn			See Section 18.2.2.2 (Architectural Perforamnce Monitoring
 *	 0-3 Thread	RW		Version 3 Facilities).  Bits 63:32 are reserved.  Note that bit
 *       4-7 Core       RW		17 enables counting Ring 0 events; we may want to restrict this.
 *       0xFFFFFFFF 0x0
 *	
 *	PERF_STATUS	RO		See Section 14.1.1.  Table 35-12 contains a duplicate 
 *	 Package			entry for this MSR.  Interpreting both, bits 0-15 are
 *	 0x0 0x0			the current performance value and 47:32 is the core 
 *					voltage: [37:32] * (float) 1/(2^13).  I have asked Intel
 *					for clarification.
 *	 
 *	PERF_CTL	RW (RMW)	Bits 15:0 are the target performance value and bit 
 *	 Thread				32 controls turbo mode (set high to disable).
 *	 0x0 0x1			Section 14.1.1 states "Applications and performance
 *					tools are not expected to use either IA32_PERF_CTL
 *					or IA32_PERF_STATUS and should treat both as reserved",
 *					but Section 14.3.2.2 states "System software can 
 *					temporarily disengage opportunistic processor performance
 *					operation by setting bit 32 of the IA32_PERF_CTL (0199H),
 *					using a read-modify-write sequence on the MSR."  
 *
 *	CLOCK_MODULATION		See 14.5.3.1 (Sandy Bridge uses the Clock Modualtion 
 *	 Thread		RW 		Extension).  Bits 4:0 are used.
 *	 0x7 0x0
 *
 * 	THERM_INTERRUPT			See 14.5.5.2.  Bits 4:0 and 24:8 are used; the rest
 *	 Core		RW		are reserved.
 *	 0x1FFFF0F	0x0
 *
 * 	THERM_STATUS			See 14.5.5.2.  
 * 	 Core		Special			Bit  Configuration
 * 	 0xAAA		0x0			00    	RO 
 * 	 					01    	R/WC0 (clear by writing 0)
 * 	 					02    	RO
 * 	 					03    	R/WC0 
 * 	 					04    	RO
 * 	 					05    	R/WC0 
 * 	 					06    	RO
 * 	 					07    	R/WC0 
 * 	 					08    	RO
 * 	 					09    	R/WC0 
 * 	 					10    	RO
 * 	 					11    	R/WC0 
 * 	 					22:16 	RO
 * 						26:23 	Reserved
 * 	 					30:27 	RO
 * 	 					31    	RO
 * 	 					63:32 	Reserved
 *
 *	MISC_ENABLE 				Just allow Speedstep and Turbo here.
 *	 0x10000 0x40
 * 	 
 * 	 Thread		RW			0 	Fast-String Enable (Section 7.3.9.3).
 * 	 					6:1	Reserved
 * 	 Thread		RO			7	Performance Monitoring Available (Section 18.4.4.3)
 *						10:8	Reserved
 *	 Thread		RO			11	Branch Trace Storage Unavailable (Section 17.4.9)
 *	 Thread		RO			12	PEBS Sampling Unavailable (Section 18.4.4.3)
 *	 					15:13	Reserved
 *	 Package	RW			16	Enable Speedstep (Section 14.1)
 *	 					17	???
 *	 Thread		RW			18	Enable Monitor FSM (See Table 35-2; do not write)
 *	 					21:19	Reserved
 *	 Thread		RW			22	Limit CPUID Maxval (See Table 35-2; do not write)
 *	 Thread		RW			23	xTPR Message Disable
 *	 					24:33	Reserved
 *	 Thread		RW			34	XD Bit Disable (See Table 35-2; do not write)
 *	 					37:35	Reserved
 *	 Package	RW			38	Turbo Mode Disable (Section 14.3.2.1)
 *	 					63:39	Reserved
 *
 *	OFFCORE_RSP_0/1				Off-core Response Performance Monitoring.
 *	 Thread		RW			See section 18.9.5.  Bits 37:15 and 11:0 are used.
 *	 0xFFFF8FFF 0x3F
 *
 *	ENERGY_PERF_BIAS			Section 14.3.4.  Used bits 3:0.
 *	 Package	RW
 *	 0xF 0x0
 *
 *	PACKAGE_THERM_STATUS			See section 14.6.  
 *	 Package	RW			This is another complex one with serveral RWC0 bits.	
 * 	0x00000555 0x0				Bits 1, 3, 5, 7, 9 and 11 are W or WC0.
 *
 * 	PACKAGE_THERM_INTERRUPT			See section 14.6
 * 	 Package 	RW			Bits 2:0, 4, and 24:8 are writable, but bit 4 controls
 * 	0x01FFFF07 0x0				generation of the critical temperature interrupt, so
 * 	 					we'll leave that out.
 *
 *	FIXED_CTRn				See section 18.2.2.  
 * 	 Thread		RW			
 * 	0xFFFFFFFF 0xFFFFFFFF
 *
 * 	PERF_CAPABILITIES			LBR and PEBS record formats, 12:0.
 * 	 Thread		RO			Section 17.4.1, others.
 *
 *      FIXED_CTR_CTRL				Section 18.2.2.1; bits 1:0, 5:3, 9:7 and 11.			
 *	 Thread         RW
 *	 0xBBB 0x0
 *	
 *	PERF_GLOBAL_STATUS			Section 18.4.2, bits 1:0, 34:32, 63:62.
 *	 Thread		RW
 *	 0x3 0xC0000007
 *
 *	PERF_GLOBAL_CTRL			Section 18.4.2, bits 1:0, 34:32.
 *	 Thread	        RW
 *	 0x3 0x7
 * 
 * 	PERF_GLOBAL_OVF_CTRL			Section 18.4.2, bits 1:0, 34:32, 63:62
 * 	 Thread		RW
 * 	 0x3 0xC0000007
 *
 * 	PEBS_ENABLE				Section 18.7.1.1, bits 3:0, 35:32
 * 	 Thread		RW
 * 	 0xF 0xF
 *
 *	PEBS_LD_LAT				Section 18.7.1.2, bits 15:0.
 *	 Thread		RW
 *	 0xFFFF 0x0
 */	 

/*	    Name		       Address  Low   	      High
 *	    					Write	      Write
 *	    					Mask	      Mask        */
#define SMSR_ENTRIES \
SMSR_ENTRY( NO_SUCH_SMSR,		{0x000, 0x0,        0x0        }),\
SMSR_ENTRY( SMSR_TIME_STAMP_COUNTER,	{0x010,	0x0,        0x0        }),\
SMSR_ENTRY( SMSR_PLATFORM_ID,		{0x017,	0x0,        0x0        }),\
SMSR_ENTRY( SMSR_PMC0,			{0x0C1,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_PMC1,			{0x0C2,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_PMC2,			{0x0C3,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_PMC3,			{0x0C4,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_PMC4,			{0x0C5,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_PMC5,			{0x0C6,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_PMC6,			{0x0C7,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_PMC7,			{0x0C8,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_MPERF,			{0x0E7,	0x0,        0x0        }),\
SMSR_ENTRY( SMSR_APERF,			{0x0E8,	0x0,        0x0        }),\
SMSR_ENTRY( SMSR_PERFEVTSEL0,		{0x186,	0xFFFFFFFF, 0x0        }),\
SMSR_ENTRY( SMSR_PERFEVTSEL1,		{0x187,	0xFFFFFFFF, 0x0        }),\
SMSR_ENTRY( SMSR_PERFEVTSEL2,		{0x188,	0xFFFFFFFF, 0x0        }),\
SMSR_ENTRY( SMSR_PERFEVTSEL3,		{0x189,	0xFFFFFFFF, 0x0        }),\
SMSR_ENTRY( SMSR_PERFEVTSEL4,		{0x18A,	0xFFFFFFFF, 0x0        }),\
SMSR_ENTRY( SMSR_PERFEVTSEL5,		{0x18B,	0xFFFFFFFF, 0x0        }),\
SMSR_ENTRY( SMSR_PERFEVTSEL6,		{0x18C,	0xFFFFFFFF, 0x0        }),\
SMSR_ENTRY( SMSR_PERFEVTSEL7,		{0x18D,	0xFFFFFFFF, 0x0        }),\
SMSR_ENTRY( SMSR_PERF_STATUS,		{0x198,	0x0,        0x0        }),\
SMSR_ENTRY( SMSR_PERF_CTL,		{0x199,	0x0       , 0x00000001 }),\
SMSR_ENTRY( SMSR_CLOCK_MODULATION,	{0x19A,	0x00000007, 0x0        }),\
SMSR_ENTRY( SMSR_THERM_INTERRUPT,	{0x19B,	0x01FFFF0F, 0x0        }),\
SMSR_ENTRY( SMSR_THERM_STATUS,		{0x19C,	0x00000AAA, 0x0        }),\
SMSR_ENTRY( SMSR_MISC_ENABLE,		{0x1A0,	0x00010000, 0x00000040 }),\
SMSR_ENTRY( SMSR_OFFCORE_RSP_0,		{0x1A6,	0xFFFF8FFF, 0x0000003F }),\
SMSR_ENTRY( SMSR_OFFCORE_RSP_1,		{0x1A7,	0xFFFF8FFF, 0x0000003F }),\
SMSR_ENTRY( SMSR_ENERGY_PERF_BIAS,	{0x1B0,	0xF,        0x0        }),\
SMSR_ENTRY( SMSR_PACKAGE_THERM_STATUS,	{0x1B1,	0x00000555, 0x0        }),\
SMSR_ENTRY( SMSR_PACKAGE_THERM_INTERRUPT,{0x1B2,0x01FFFF07, 0x0        }),\
SMSR_ENTRY( SMSR_FIXED_CTR0,		{0x309,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_FIXED_CTR1,		{0x30A,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_FIXED_CTR2,		{0x30A,	0xFFFFFFFF, 0xFFFFFFFF }),\
SMSR_ENTRY( SMSR_PERF_CAPABILITIES,	{0x345,	0x0,        0x0        }),\
SMSR_ENTRY( SMSR_FIXED_CTR_CTRL,	{0x38D,	0x00000BBB, 0x0        }),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_STATUS,	{0x38E,	0x00000003, 0xC0000007 }),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_CTRL,	{0x38F,	0x00000003, 0x00000007 }),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_OVF_CTRL,	{0x390,	0x00000003, 0xC0000007 }),\
SMSR_ENTRY( SMSR_PEBS_ENABLE,		{0x3F1,	0x0000000F, 0x0000000F }),\
SMSR_ENTRY( SMSR_PEBS_LD_LAT,		{0x3F6,	0x0000FFFF, 0x0        }),\
SMSR_ENTRY( SMSR_RAPL_POWER_UNIT,	{0x606,	SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_PKG_POWER_LIMIT,	{0x610,	SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PKG_ENERGY_STATUS,	{0x611,	SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_PKG_POWER_INFO,	{0x612,	SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_PP0_POWER_LIMIT,	{0x638,	SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PP0_ENERGY_STATUS,	{0x639,	SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_MSR_PKG_PERF_STATUS,	{0x613,	SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_DRAM_POWER_LIMIT,	{0x618,	SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_DRAM_ENERGY_STATUS,	{0x619,	SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_DRAM_PERF_STATUS,	{0x61B,	SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_DRAM_POWER_INFO,	{0x61C,	SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_LAST_ENTRY, 		{0x000, 0x0,           0x0          })

#endif //_USE_ARCH_062D

#endif /* MSR_SUPPLEMENTAL_H */
