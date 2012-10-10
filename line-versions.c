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
static pixel *maximum_2(int dim, pixel *src)
{
    int jj;
    pixel *maxi = src;
    int darkness = 0;
    int tot = dim * dim;

    for(jj = 0; jj < tot; jj++)
    {
        if (intensity (src[jj]) > darkness)
        {
            darkness = intensity (src[jj]);
            maxi = &(src[jj]);
        }
    }
    return maxi;
}

static pixel *maximum_3(int dim, pixel *src)
{
    int jj;
    pixel *maxi = src;
    int darkness = 0;
    int tot = dim * dim;

    for(jj = 0; jj < tot; jj++)
    {
        int inten = src[jj].green + src[jj].red + src[jj].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj]);
        }
    }
    return maxi;
}

static pixel *maximum_4(int dim, pixel *src)
{
    int jj;
    pixel *maxi = src;
    int darkness = 0;
    int tot = dim * dim;

    for(jj = 0; jj < tot; jj+=8)
    {
        int inten = src[jj].green + src[jj].red + src[jj].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj]);
        }
        inten = src[jj+1].green + src[jj+1].red + src[jj+1].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj+1]);
        }
        inten = src[jj+2].green + src[jj+2].red + src[jj+2].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj+2]);
        }
        inten = src[jj+3].green + src[jj+3].red + src[jj+3].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj+3]);
        }
        inten = src[jj+4].green + src[jj+4].red + src[jj+4].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj+4]);
        }
        inten = src[jj+5].green + src[jj+5].red + src[jj+5].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj+5]);
        }
        inten = src[jj+6].green + src[jj+6].red + src[jj+6].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj+6]);
        }
        inten = src[jj+7].green + src[jj+7].red + src[jj+7].blue;
        if (inten > darkness)
        {
            darkness = inten;
            maxi = &(src[jj+7]);
        }
    }
    return maxi;
}

static pixel *maximum_5(int dim, pixel *src)
{
    int jj;
    pixel *maxi = src;
    int darkness = 0;
    int tot = dim * dim;

    for(jj = 0; jj < tot; jj+=8)
    {
        if (src[jj].green+src[jj].red+src[jj].blue > darkness)
        {
            darkness = src[jj].green+src[jj].red+src[jj].blue;
            maxi = &(src[jj]);
        }
        if (src[jj+1].green+src[jj+1].red+src[jj+1].blue > darkness)
        {
            darkness = src[jj+1].green+src[jj+1].red+src[jj+1].blue;
            maxi = &(src[jj+1]);
        }
        if (src[jj+2].green+src[jj+2].red+src[jj+2].blue > darkness)
        {
            darkness = src[jj+2].green+src[jj+2].red+src[jj+2].blue;
            maxi = &(src[jj+2]);
        }
        if (src[jj+3].green+src[jj+3].red+src[jj+3].blue > darkness)
        {
            darkness = src[jj+3].green+src[jj+3].red+src[jj+3].blue;
            maxi = &(src[jj+3]);
        }
        if (src[jj+4].green+src[jj+4].red+src[jj+4].blue > darkness)
        {
            darkness = src[jj+4].green+src[jj+4].red+src[jj+4].blue;
            maxi = &(src[jj+4]);
        }
        if (src[jj+5].green+src[jj+5].red+src[jj+5].blue > darkness)
        {
            darkness = src[jj+5].green+src[jj+5].red+src[jj+5].blue;
            maxi = &(src[jj+5]);
        }
        if (src[jj+6].green+src[jj+6].red+src[jj+6].blue > darkness)
        {
            darkness = src[jj+6].green+src[jj+6].red+src[jj+6].blue;
            maxi = &(src[jj+6]);
        }
        if (src[jj+7].green+src[jj+7].red+src[jj+7].blue > darkness)
        {
            darkness = src[jj+7].green+src[jj+7].red+src[jj+7].blue;
            maxi = &(src[jj+7]);
        }
    }
    return maxi;
}

static pixel *maximum_6(int dim, pixel *src)
{
    int jj;
    pixel *maxi = src;
    int darkness = 0;
    int tot = dim * dim;
    pixel px, px1, px2, px3, px4, px5, px6, px7;
    int it, it1, it2, it3, it4, it5, it6, it7;

    for(jj = 0; jj < tot; jj+=8)
    {
        px = src[jj];
        px1 = src[jj+1];
        px2 = src[jj+2];
        px3 = src[jj+3];
        px4 = src[jj+4];
        px5 = src[jj+5];
        px6 = src[jj+6];
        px7 = src[jj+7];
        it = px.green + px.red + px.blue;
        it1 = px1.green + px1.red + px1.blue;
        it2 = px2.green + px2.red + px2.blue;
        it3 = px3.green + px3.red + px3.blue;
        it4 = px4.green + px4.red + px4.blue;
        it5 = px5.green + px5.red + px5.blue;
        it6 = px6.green + px6.red + px6.blue;
        it7 = px7.green + px7.red + px7.blue;

        if (it > darkness)
        {
            darkness = it;
            maxi = &(src[jj]);
        }
        if (it1 > darkness)
        {
            darkness = it1;
            maxi = &(src[jj+1]);
        }
        if (it2 > darkness)
        {
            darkness = it2;
            maxi = &(src[jj+2]);
        }
        if (it3 > darkness)
        {
            darkness = it3;
            maxi = &(src[jj+3]);
        }
        if (it4 > darkness)
        {
            darkness = it4;
            maxi = &(src[jj+4]);
        }
        if (it5 > darkness)
        {
            darkness = it5;
            maxi = &(src[jj+5]);
        }
        if (it6 > darkness)
        {
            darkness = it6;
            maxi = &(src[jj+6]);
        }
        if (it7 > darkness)
        {
            darkness = it7;
            maxi = &(src[jj+7]);
        }
    }
    return maxi;
}

static pixel *maximum_7(int dim, pixel *src)
{
    int jj;
    pixel *maxi = src;
    int darkness = 0;
    int tot = dim * dim;
    pixel px, px1, px2, px3, px4, px5, px6, px7;

    for(jj = 0; jj < tot; jj+=8)
    {
        px = src[jj];
        px1 = src[jj+1];
        px2 = src[jj+2];
        px3 = src[jj+3];
        px4 = src[jj+4];
        px5 = src[jj+5];
        px6 = src[jj+6];
        px7 = src[jj+7];
        
	if (px.green + px.red + px.blue > darkness)
        {
            darkness = px.green + px.red + px.blue;
            maxi = &(src[jj]);
        }
        if (px1.green + px1.red + px1.blue > darkness)
        {
            darkness = px1.green + px1.red + px1.blue;
            maxi = &(src[jj+1]);
        }
        if (px2.green + px2.red + px2.blue > darkness)
        {
            darkness = px2.green + px2.red + px2.blue;
            maxi = &(src[jj+2]);
        }
        if (px3.green + px3.red + px3.blue > darkness)
        {
            darkness = px3.green + px3.red + px3.blue;
            maxi = &(src[jj+3]);
        }
        if (px4.green + px4.red + px4.blue > darkness)
        {
            darkness = px4.green + px4.red + px4.blue; 
            maxi = &(src[jj+4]);
        }
        if (px5.green + px5.red + px5.blue > darkness)
        {
            darkness = px5.green + px5.red + px5.blue ;
            maxi = &(src[jj+5]);
        }
        if (px6.green + px6.red + px6.blue > darkness)
        {
            darkness = px6.green + px6.red + px6.blue;
            maxi = &(src[jj+6]);
        }
        if (px7.green + px7.red + px7.blue > darkness)
        {
            darkness = px7.green + px7.red + px7.blue ;
            maxi = &(src[jj+7]);
        }
        
    }
    return maxi;
}
/******************************************************
 * Your different versions of the draw_line function go here
 ******************************************************/

/* 
 * naive_version - The naive baseline version of draw_line
 */

void naive_line(int dim, pixel *src, pixel *dst) {
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

void line_1(int dim, pixel *src, pixel *dst) {
    int x0 = 0;
    int y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1;
    int y1 = ceil (dim - 1 - dim / 3); /* right endpoint */

    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;

    double y = y0;
    int x = x0;
    pixel max = *maximum(dim, src);

    for (; x <= x1; x++) {
        dst[RIDX(x, (int)rint(y), dim)] = max;
        y += slope;
    }
}

void line_2(int dim, pixel *src, pixel *dst) {
    int x0 = 0;
    int y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1;
    int y1 = ceil (dim - 1 - dim / 3); /* right endpoint */

    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;

    double y = y0;
    int x = x0;
    pixel max = *maximum_2(dim, src);

    for (; x <= x1; x++) {
        dst[RIDX(x, (int)rint(y), dim)] = max;
        y += slope;
    }
}

void line_3(int dim, pixel *src, pixel *dst) {
    int x0 = 0;
    int y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1;
    int y1 = ceil (dim - 1 - dim / 3); /* right endpoint */

    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;

    double y = y0;
    int x = x0;
    pixel max = *maximum_3(dim, src);

    for (; x <= x1; x++) {
        dst[RIDX(x, (int)rint(y), dim)] = max;
        y += slope;
    }
}

void line_4(int dim, pixel *src, pixel *dst) {
    int x0 = 0;
    int y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1;
    int y1 = ceil (dim - 1 - dim / 3); /* right endpoint */

    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;

    double y = y0;
    int x = x0;
    pixel max = *maximum_4(dim, src);

    for (; x <= x1; x++) {
        dst[RIDX(x, (int)rint(y), dim)] = max;
        y += slope;
    }
}

void line_5(int dim, pixel *src, pixel *dst) {
    int x0 = 0;
    int y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1;
    int y1 = ceil (dim - 1 - dim / 3); /* right endpoint */

    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;

    double y = y0;
    int x = x0;
    pixel max = *maximum_5(dim, src);

    for (; x <= x1; x++) {
        dst[RIDX(x, (int)rint(y), dim)] = max;
        y += slope;
    }

}

void line_6(int dim, pixel *src, pixel *dst) {
    int x0 = 0;
    int y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1;
    int y1 = ceil (dim - 1 - dim / 3); /* right endpoint */

    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;
    double slope2 = slope*2;
    double slope3 = slope*3;
    double slope4 = slope*4;
    double slope5 = slope*5;
    double slope6 = slope*6;
    double slope7 = slope*7;
    double slope8 = slope*8;

    double y = y0;
    int x = x0;
    pixel max = *maximum_6(dim, src);

    for (; x <= x1; x+=8) {
        dst[RIDX(x, (int)rint(y), dim)] = max;
        dst[RIDX(x+1, (int)rint(y+slope), dim)] = max;
        dst[RIDX(x+2, (int)rint(y+slope2), dim)] = max;
        dst[RIDX(x+3, (int)rint(y+slope3), dim)] = max;
        dst[RIDX(x+4, (int)rint(y+slope4), dim)] = max;
        dst[RIDX(x+5, (int)rint(y+slope5), dim)] = max;
        dst[RIDX(x+6, (int)rint(y+slope6), dim)] = max;
        dst[RIDX(x+7, (int)rint(y+slope7), dim)] = max;
        y += slope8;
    }

}

void line_7(int dim, pixel *src, pixel *dst) {
    int x0 = 0;
    int y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1;
    int y1 = ceil (dim - 1 - dim / 3); /* right endpoint */

    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;
    double slope2 = slope*2;
    double slope3 = slope*3;
    double slope4 = slope*4;
    double slope5 = slope*5;
    double slope6 = slope*6;
    double slope7 = slope*7;
    double slope8 = slope*8;

    double y = y0;
    int x = x0;
    pixel max = *maximum_7(dim, src);

    for (; x <= x1; x+=8) {
        dst[RIDX(x, (int)rint(y), dim)] = max;
        dst[RIDX(x+1, (int)rint(y+slope), dim)] = max;
        dst[RIDX(x+2, (int)rint(y+slope2), dim)] = max;
        dst[RIDX(x+3, (int)rint(y+slope3), dim)] = max;
        dst[RIDX(x+4, (int)rint(y+slope4), dim)] = max;
        dst[RIDX(x+5, (int)rint(y+slope5), dim)] = max;
        dst[RIDX(x+6, (int)rint(y+slope6), dim)] = max;
        dst[RIDX(x+7, (int)rint(y+slope7), dim)] = max;
        y += slope8;
    }

}
