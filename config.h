/*********************************************************
 * config.h - Configuration data for the driver.c program.
 *********************************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_

/* 
 * CPEs for the baseline (naive) version of the rotate function.
 * Rd is the measured CPE for a dxd image.
 * Measured on a Pentium 4 processor (CPUID signature 0F27h).
 */
#define R64    12
#define R128   20
#define R256   45
#define R512   125
#define R1024  145

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
 * L64 measured on a Pentium Dual processor E2180.
 * New measurements for an 'Intel(R) Core(TM) i3-2120 CPU @ 3.30GHz'
 */
#define L64    1102  
#define L128   2482 
#define L256   6108
#define L512   14068
#define L1024  26999 
#define L1536  45185

#endif /* _CONFIG_H_ */
