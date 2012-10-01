/********************************************************
 * The baseline Rotate function to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "rotate-versions.h"

/***************
 * ROTATE KERNEL
 ***************/

/* 
 * baseline_rotate - Equal to the naive baseline version of rotate 
 */
void baseline_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}
