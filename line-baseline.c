/********************************************************
 * Line-draw function to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "line-versions.h"

/***************
 * DRAW-LINE KERNEL
 ***************/

/***************************************************************
* Various typedefs and helper functions for the draw_line function
* You may modify these any way you like.
**************************************************************/

static int intensity (pixel src)
{
        return src.green + src.red + src.blue;
}

/*
* maximum - Returns the adress of the pixel with the highest intensity in src
*/
static pixel *maximum(int dim, pixel *src)
{
    int ii, jj;
    pixel *maxi = src;
    int darkness = 0;

    for(jj = dim-1; jj >= 0; jj--)
        for(ii = 0; ii <= dim-1; ii++)
            if (intensity (src[RIDX(ii, jj, dim)]) > darkness)
            {
                darkness = intensity (src[RIDX(ii, jj, dim)]);
                maxi = &(src[RIDX(ii, jj, dim)]);
            }

    return maxi;
}

/******************************************************
 * Your different versions of the draw_line function go here
 ******************************************************/

/* 
 * naive_version - The naive baseline version of draw_line
 */

void baseline_line(int dim, pixel *src, pixel *dst) {
    int x0 = 0;
    int y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1;
    int y1 = ceil (dim - 1 - dim / 3); /* right endpoint */

    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;

    double y = y0;
    int x = x0;

    for (; x <= x1; x++) {
        dst[RIDX(x,(int )rint(y), dim)] = *maximum(dim, src);
        y += slope;
    }
}

