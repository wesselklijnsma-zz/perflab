/********************************************************
 * Functions to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "line-versions.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "vdmaasklijnsma",              /* Team name */

    "Jouke van der Maas",     /* First member full name */
    "jouke.vandermaas@gmail.com",  /* First member email address */

    "Wessel Klijnsma",                   /* Second member full name (leave blank if none) */
    "wesselklijnsma@gmail.com"                    /* Second member email addr (leave blank if none) */
};

/*********************************************************************
 * line - Your current working version of draw_line
 * IMPORTANT: In 2011 you WILL be graded on this function
 *********************************************************************/

char line_descr[] = "line: Current working version";
void line(int dim, pixel *src, pixel *dst)
{
   line_4(dim, src, dst);
}

/*********************************************************************
 * register_line_functions - Register all of your different versions
 *     of the draw_line function to the driver by calling the
 *     add_line_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_line_functions() 
{
    /* ... the current version is test first */
    add_line_function(&line, line_descr);  
    /* ... Register intermediate results here */
    /* ... Remove the naive implementation as fast as possible,
           because testing this version takes a lot of time */
    add_line_function(&line_3, LINE_2_DESCR);
    //add_line_function(&naive_line, NAIVE_LINE_DESCR);  
    set_line_baseline(LINE_3_DESCR);
}

/*********************************************************************
 * rotate - Your current working version 
 * IMPORTANT: In 2011 you NOT be graded on this function
 *********************************************************************/

/*
 * rotate - Your current working version of rotate
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst)
{
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_rotate_functions()
{

    /* add_rotate_function (&rotate, rotate_descr); */
    /* ... Register additional test functions here */
    /* add_rotate_function (&naive_rotate, NAIVE_ROTATE_DESCR); */

}

/*********************************************************************
 * IMPORTANT: In 2011 you NOT be graded on this function
 *********************************************************************/
/*
 * smooth - Your current working version of smooth.
 */
char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst)
{
}

/*********************************************************************
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_smooth_functions() {
    /* ... Register additional test functions here */
}
