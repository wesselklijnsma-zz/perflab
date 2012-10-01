/*********************************************************
 * config.h - Configuration data for the driver.c program.
 *********************************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_

/* 
 * CPEs for the baseline (naive) version of the rotate function.
 * Rd is the measured CPE for a dxd image.
 * Measured on a Pentium E2180 processor (CPUID signature 06FDh).
 */
#define R64    16
#define R128   12
#define R256   12
#define R512   30
#define R1024  60
#define R1536  99
#define R2048 107

/* 
 * CPEs for the baseline (naive) version of the smooth function
 * Sd is the measured CPE for a dxd image.
 * Measured on a Pentium 4 processor.
 */
#define S32    690
#define S64    694
#define S128   705
#define S256   725
#define S512   726
#define S1024  731

/* 
 * CPEs for the baseline (naive) version of the line function
 * Sd is the measured CPE for a dxd image.
 * L64 and L128 Measured on a Pentium 4 processor.
 * larger images measured on a Xeon Processor
 */
#define L64    72  
#define L128   74  
#define L256   78
#define L512   190
#define L1024  260   
#define L1536  260
#define L2048  260

#endif /* _CONFIG_H_ */
