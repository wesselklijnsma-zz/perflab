/*******************************************************************
 * 
 * driver.c - Driver program for CS:APP Performance Lab
 * 
 * In kernels.c, students generate an arbitrary number of rotate and
 * smooth test functions, which they then register with the driver
 * program using the add_rotate_function() and add_smooth_function()
 * functions.
 * 
 * The driver program runs and measures the registered test functions
 * and reports their performance.
 * 
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights
 * reserved.  May not be used, modified, or copied without permission.
 *
 ********************************************************************/

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "fcyc.h"
#include "cpe.h"
#include "defs.h"
#include "config.h"
#include "line-baseline.h"

/* Team structure that identifies the students */
extern team_t team; 

/* Keep track of a number of different test functions */
#define MAX_BENCHMARKS 100
#define DIM_CNT 5

/* Misc constants */
#define BSIZE 32     /* cache block size in bytes */     
#define MAX_DIM 2304 /* 1024 + 256 */
#define ODD_DIM 96   /* not a power of 2 */

#define OLD 

/* fast versions of min and max */
#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

/* This struct characterizes the results for one benchmark test */
typedef struct {
    lab_test_func tfunct; /* The test function */
    double cpes[DIM_CNT]; /* One CPE result for each dimension */
    char *description;    /* ASCII description of the test function */
    unsigned short valid; /* The function is tested if this is non zero */
} bench_t;

/* The range of image dimensions that we will be testing */
/* static int test_dim_rotate[] = {64, 128, 256, 512, 1024}; */
static int test_dim_rotate[] = {256, 512, 1024, 1536, 2048};
/* static int test_dim_smooth[] = {32, 64, 128, 256, 512}; */
static int test_dim_smooth[] =  {64, 128, 256, 512, 1024};
/* static int test_dim_line[] =  {64, 128, 256, 512, 1024}; */
static int test_dim_line[] =  {128, 256, 512, 1024, 1536}; 
/* static int test_dim_line[] =  {256, 512, 1024, 1536, 2048}; */

/* Baseline CPEs (see config.h) */
/* static double rotate_baseline_cpes[] = {R64, R128, R256, R512, R1024}; */
static double rotate_baseline_cpes[] = {R256, R512, R1024, R1536, R2048}; 
/* static double smooth_baseline_cpes[] = {S32, S64, S128, S256, S512}; */
static double smooth_baseline_cpes[] = {S64, S128, S256, S512, S1024};
/* static double line_baseline_cpes[] = {L64, L128, L256, L512, L1024}; */
static double line_baseline_cpes[] = {L256, L512, L1024, L1536, L2048};

/* These hold the results for all benchmarks */
static bench_t benchmarks_rotate[MAX_BENCHMARKS];
static bench_t benchmarks_smooth[MAX_BENCHMARKS];
static bench_t benchmarks_line[MAX_BENCHMARKS];

/* These give the sizes of the above lists */
static int rotate_benchmark_count = 0;
static int smooth_benchmark_count = 0;
static int line_benchmark_count = 0;

/* This is the counter used as intermediate baseline */
static int line_benchmark_base = -1; /* not defined */
static int autograder = 0;

/* 
 * An image is a dimxdim matrix of pixels stored in a 1D array.  The
 * data array holds three images (the input original, a copy of the original, 
 * and the output result array. There is also an additional BSIZE bytes
 * of padding for alignment to cache block boundaries.
 */
static pixel data[(3*MAX_DIM*MAX_DIM) + (BSIZE/sizeof(pixel))];

/* Various image pointers */
static pixel *orig = NULL;         /* original image */
static pixel *copy_of_orig = NULL; /* copy of original for checking result */
static pixel *result = NULL;       /* result image */

/* Keep track of the best rotate and smooth score for grading */
double rotate_maxmean = 0.0;
char *rotate_maxmean_desc = NULL;

double smooth_maxmean = 0.0;
char *smooth_maxmean_desc = NULL;

double line_maxmean = 0.0;
char *line_maxmean_desc = NULL;


/******************** Functions begin *************************/

void add_smooth_function(lab_test_func f, char *description) 
{
    benchmarks_smooth[smooth_benchmark_count].tfunct = f;
    benchmarks_smooth[smooth_benchmark_count].description = description;
    benchmarks_smooth[smooth_benchmark_count].valid = 0;  
    smooth_benchmark_count++;
}


void add_rotate_function(lab_test_func f, char *description) 
{
    benchmarks_rotate[rotate_benchmark_count].tfunct = f;
    benchmarks_rotate[rotate_benchmark_count].description = description;
    benchmarks_rotate[rotate_benchmark_count].valid = 0;
    rotate_benchmark_count++;
}

void add_line_function(lab_test_func f, char *description) 
{
    benchmarks_line[line_benchmark_count].tfunct = f;
    benchmarks_line[line_benchmark_count].description = description;
    benchmarks_line[line_benchmark_count].valid = 0;
    line_benchmark_count++;
}

int set_line_baseline(char *description)
{
    int i;
    for (i=0;i<line_benchmark_count;i++)
    {
        if (strcmp(benchmarks_line[i].description,description) == 0)
        {
            line_benchmark_base = i;
#ifdef DEBUG
        printf("DEBUG: Setting baseline to \"%s\"\n", benchmarks_line[bench_index].description);
#endif
         }
    }
    return line_benchmark_base;
}
/* 
 * random_in_interval - Returns random integer in interval [low, high) 
 */
static int random_in_interval(int low, int high) 
{
    int size = high - low;
    return (rand()% size) + low;
}

/*
 * create - creates a dimxdim image aligned to a BSIZE byte boundary
 */
static void create(int dim)
{
    int i, j;

    /* Align the images to BSIZE byte boundaries */
    orig = data;
    while ((unsigned long)orig % BSIZE)
        orig=(pixel *)(((char *)orig)+1);
    result = orig + dim*dim;
    copy_of_orig = result + dim*dim;

    for (i = 0; i < dim; i++) {
	for (j = 0; j < dim; j++) {
	    /* Original image initialized to random colors */
	    orig[RIDX(i,j,dim)].red = random_in_interval(0, 65536);
	    orig[RIDX(i,j,dim)].green = random_in_interval(0, 65536);
	    orig[RIDX(i,j,dim)].blue = random_in_interval(0, 65536);

	    /* Copy of original image for checking result */
	    copy_of_orig[RIDX(i,j,dim)].red = orig[RIDX(i,j,dim)].red;
	    copy_of_orig[RIDX(i,j,dim)].green = orig[RIDX(i,j,dim)].green;
	    copy_of_orig[RIDX(i,j,dim)].blue = orig[RIDX(i,j,dim)].blue;

	    /* Result image initialized to all black */
	    result[RIDX(i,j,dim)].red = 0;
	    result[RIDX(i,j,dim)].green = 0;
	    result[RIDX(i,j,dim)].blue = 0;
	}
    }

    return;
}


/* 
 * compare_pixels - Returns 1 if the two arguments don't have same RGB
 *    values, 0 o.w.  
 */
static int compare_pixels(pixel p1, pixel p2) 
{
    return 
	(p1.red != p2.red) || 
	(p1.green != p2.green) || 
	(p1.blue != p2.blue);
}


/* Make sure the orig array is unchanged */
static int check_orig(int dim) 
{
    int i, j;

    for (i = 0; i < dim; i++) 
	for (j = 0; j < dim; j++) 
	    if (compare_pixels(orig[RIDX(i,j,dim)], copy_of_orig[RIDX(i,j,dim)])) {
		printf("\n");
		printf("Error: Original image of dimension %d has been changed at (%d,%d)!\n",dim,i,j);
		return 1;
	    }

    return 0;
}

/* 
 * check_rotate - Make sure the rotate actually works. 
 * The orig array should not  have been tampered with! 
 */
static int check_rotate(int dim) 
{
    int err = 0;
    int i, j;
    int badi = 0;
    int badj = 0;
    pixel orig_bad = {-1,-1,-1};
    pixel res_bad = {-2,-2,-2};

    /* return 1 if the original image has been  changed */
    if (check_orig(dim)) 
	return 1; 

    for (i = 0; i < dim; i++) 
	for (j = 0; j < dim; j++) 
	    if (compare_pixels(orig[RIDX(i,j,dim)], 
			       result[RIDX(dim-1-j,i,dim)])) {
		err++;
		badi = i;
		badj = j;
		orig_bad = orig[RIDX(i,j,dim)];
		res_bad = result[RIDX(dim-1-j,i,dim)];
	    }

    if (err) {
	printf("\n");
	printf("ERROR: Dimension=%d, %d errors\n", dim, err);    
	printf("E.g., The following two pixels should have equal value:\n");
	printf("src[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       badi, badj, orig_bad.red, orig_bad.green, orig_bad.blue);
	printf("dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       (dim-1-badj), badi, res_bad.red, res_bad.green, res_bad.blue);
    }

    return err;
}

static pixel check_average(int dim, int i, int j, pixel *src) {
    pixel result;
    int num = 0;
    int ii, jj;
    int sum0, sum1, sum2;
    int top_left_i, top_left_j;
    int bottom_right_i, bottom_right_j;

    top_left_i = max(i-1, 0);
    top_left_j = max(j-1, 0);
    bottom_right_i = min(i+1, dim-1); 
    bottom_right_j = min(j+1, dim-1);

    sum0 = sum1 = sum2 = 0;
    for(ii=top_left_i; ii <= bottom_right_i; ii++) {
	for(jj=top_left_j; jj <= bottom_right_j; jj++) {
	    num++;
	    sum0 += (int) src[RIDX(ii,jj,dim)].red;
	    sum1 += (int) src[RIDX(ii,jj,dim)].green;
	    sum2 += (int) src[RIDX(ii,jj,dim)].blue;
	}
    }
    result.red = (unsigned short) (sum0/num);
    result.green = (unsigned short) (sum1/num);
    result.blue = (unsigned short) (sum2/num);
 
    return result;
}


/* 
 * check_smooth - Make sure the smooth function actually works.  The
 * orig array should not have been tampered with!  
 */
static int check_smooth(int dim) {
    int err = 0;
    int i, j;
    int badi = 0;
    int badj = 0;
    pixel right = {0,0,0};
    pixel wrong = {-1,-1,-1};
    /* return 1 if original image has been changed */
    if (check_orig(dim)) 
	return 1; 

    for (i = 0; i < dim; i++) {
	for (j = 0; j < dim; j++) {
	    pixel smoothed = check_average(dim, i, j, orig);
	    if (compare_pixels(result[RIDX(i,j,dim)], smoothed)) {
		err++;
		badi = i;
		badj = j;
		wrong = result[RIDX(i,j,dim)];
		right = smoothed;
	    }
	}
    }

    if (err) {
	printf("\n");
	printf("ERROR: Dimension=%d, %d errors\n", dim, err);    
	printf("E.g., \n");
	printf("You have dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       badi, badj, wrong.red, wrong.green, wrong.blue);
	printf("It should be dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       badi, badj, right.red, right.green, right.blue);
    }

    return err;
}

static int check_line(int dim) {

    int x,y;
    int err = 0;
    int badi = 0;
    int badj = 0;
    pixel right = { 0, 0, 0};
    pixel wrong = {-1,-1,-1};
    pixel *black = orig;
    pixel *current = orig;
    int darkness = 0;

    int x0 = 0, y0 = floor (dim / 3); /* left endpoint */
    int x1 = dim - 1, y1 = ceil (dim - 1 - dim / 3); /* right endpoint */
                                                                                
    double dy = y1 - y0;
    double dx = x1 - x0;
    double slope = dy / dx;
                                                                                
    double ry = y0;
#if 1
    /* return 1 if original image has been changed */
    if (check_orig(dim))
        return 1;
#endif
#if 1
    for(x = 0; x <= dim-1; x++) {
       for(y = 0; y <= dim-1; y++) {
            if (current->green + current->red + current->blue > darkness)
            {
                darkness = current->green + current->red + current->blue;
                black = current;
            }
            current++;
        }
     }
#endif

    for (x= 0; x <= dim-1; x++) {
        if (compare_pixels (result[RIDX(x,(int )rint(ry), dim)], *black)) {

                err++;
                badi = x;
                badj = rint(ry);
                wrong = result[RIDX(x,badj,dim)];
                right = *black;
         }
 
        if (err == 1) {
            printf("\n");
            printf("ERROR: Dimension=%d\n", dim);
            printf("E.g., \n");
            printf("You have dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
               badi, badj, wrong.red, wrong.green, wrong.blue);
            printf("It should be dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
               badi, badj, right.red, right.green, right.blue);
        }

        ry += slope;
    }

    if (err > 1) {
        printf("\n");
        printf("ERROR: Dimension=%d, %d errors\n", dim, err);
        printf("E.g., \n");
        printf("You have dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
               badi, badj, wrong.red, wrong.green, wrong.blue);
        printf("It should be dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
               badi, badj, right.red, right.green, right.blue);
    }

    return err;
}

void func_wrapper(void *arglist[]) 
{
    pixel *src, *dst;
    int mydim;
    lab_test_func f;

    f = (lab_test_func) arglist[0];
    mydim = *((int *) arglist[1]);
    src = (pixel *) arglist[2];
    dst = (pixel *) arglist[3];

    (*f)(mydim, src, dst);

    return;
}

void run_rotate_benchmark(int idx, int dim) 
{
    benchmarks_rotate[idx].tfunct(dim, orig, result);
}

void baseline_rotate(int, pixel *, pixel *); 

#ifdef IF_NEEDED

    for(test_num=0; test_num < DIM_CNT; test_num++) {
	int dim;

	/* Check correctness for odd (non power of two dimensions */
	create(ODD_DIM);
	run_line_benchmark(bench_index, ODD_DIM);
	if (check_line(ODD_DIM)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_line[bench_index].description, ODD_DIM);
	    return;
	}

	/* Create a test image of the required dimension */
	dim = test_dim_line[test_num];
	create(dim);

#ifdef DEBUG
	printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_line[bench_index].description);
#endif
	/* Check that the code works */
	run_line_benchmark(bench_index, dim);
#endif

void test_rotate(int bench_index) 
{
    int i;
    int test_num;
    char *description = benchmarks_rotate[bench_index].description;
  
    for (test_num = 0; test_num < DIM_CNT; test_num++) {
	int dim;

	/* Check for odd dimension */
	create(ODD_DIM);
	run_rotate_benchmark(bench_index, ODD_DIM);
	if (check_rotate(ODD_DIM)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_rotate[bench_index].description, ODD_DIM);
	    return;
	}

	/* Create a test image of the required dimension */
	dim = test_dim_rotate[test_num];
	create(dim);
#ifdef DEBUG
	printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_rotate[bench_index].description);
#endif

	/* Check that the code works */
	run_rotate_benchmark(bench_index, dim);
	if (check_rotate(dim)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_rotate[bench_index].description, dim);
	    return;
	}

	/* Measure CPE */
	{
	    double cpe;
#ifdef OLD
	    double num_cycles;
#endif
	    int tmpdim = dim;
            void *arglist[4];
	    double dimension = (double) dim;
	    double work = dimension*dimension;
#ifdef DEBUG
	    printf("DEBUG: dimension=%.1f\n",dimension);
	    printf("DEBUG: work=%.1f\n",work);
#endif
	    arglist[0] = (void *) benchmarks_rotate[bench_index].tfunct;
	    arglist[1] = (void *) &tmpdim;
	    arglist[2] = (void *) orig;
	    arglist[3] = (void *) result;

	    create(dim);
#ifdef OLD
	    num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist, CLEAR_CACHE); 
	    cpe = num_cycles/work;
#else
            /* work or dimension? */
	    cpe = find_cpe_v((test_funct_v)&func_wrapper, arglist,work);
#endif
	    benchmarks_rotate[bench_index].cpes[test_num] = cpe;
	}
    }

    /* 
     * Print results as a table 
     */
    printf("Rotate: Version = %s:\n", description);
    printf("Dim\t");
    for (i = 0; i < DIM_CNT; i++)
	printf("\t%d", test_dim_rotate[i]);
    printf("\tMean\n");
  
    printf("Your CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", benchmarks_rotate[bench_index].cpes[i]);
    }
    printf("\n");

    /* Measure baseline CPE */
   for (test_num = 0; test_num < DIM_CNT; test_num++) {
        int dim;

        /* Create a test image of the required dimension */
        dim = test_dim_rotate[test_num];
        create(dim);

        double cpe;
#ifdef OLD
        double num_cycles;
#endif
        int tmpdim = dim;
        void *arglist[4];
        double dimension = (double) dim;
        double work = dimension*dimension;
#ifdef DEBUG
        printf("AVI DEBUG: dimension=%.1f\n",dimension);
#endif                    
#ifdef IA32
    for (i = 0; i < DIM_CNT; i++) {
        printf("\t%.1f", rotate_baseline_cpes[i]);
    }
#endif
        arglist[0] = (void *) (lab_test_func)baseline_rotate;
        arglist[1] = (void *) &tmpdim;
        arglist[2] = (void *) orig;
        arglist[3] = (void *) result;

        create(dim);
#ifdef OLD
        num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist, CLEAR_CACHE); 
        cpe = num_cycles/work;
#else
               /* work or dimension? */
        cpe = find_cpe_v((test_funct_v)&func_wrapper, arglist,work);
#endif
#ifdef IA32
        printf("AVI DEBUG: rotate_baseline_cpes[%d]: %.1f -> %.1f\n",test_num, rotate_baseline_cpes[test_num],cpe);
#endif
	rotate_baseline_cpes[test_num] = cpe;
    }
    printf("Baseline CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", rotate_baseline_cpes[i]);
    }
    printf("\n");

    /* Compute Speedup */
    {
	double prod, ratio, mean;
	prod = 1.0; /* Geometric mean */
	printf("Speedup\t");
	for (i = 0; i < DIM_CNT; i++) {
	    if (benchmarks_rotate[bench_index].cpes[i] > 0.0) {
		ratio = rotate_baseline_cpes[i]/
		    benchmarks_rotate[bench_index].cpes[i];
	    }
	    else {
		printf("Fatal Error: Non-positive CPE value...\n");
		exit(EXIT_FAILURE);
	    }
	    prod *= ratio;
	    printf("\t%.1f", ratio);
	}

	/* Geometric mean */
	mean = pow(prod, 1.0/(double) DIM_CNT);
	printf("\t%.1f", mean);
	printf("\n\n");
	if (mean > rotate_maxmean) {
	    rotate_maxmean = mean;
	    rotate_maxmean_desc = benchmarks_rotate[bench_index].description;
	}
    }


#ifdef DEBUG
    fflush(stdout);
#endif
    return;  
}

void run_smooth_benchmark(int idx, int dim) 
{
    benchmarks_smooth[idx].tfunct(dim, orig, result);
}

void test_smooth(int bench_index) 
{
    int i;
    int test_num;
    char *description = benchmarks_smooth[bench_index].description;
  
    for(test_num=0; test_num < DIM_CNT; test_num++) {
	int dim;

	/* Check correctness for odd (non power of two dimensions */
	create(ODD_DIM);
	run_smooth_benchmark(bench_index, ODD_DIM);
	if (check_smooth(ODD_DIM)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_smooth[bench_index].description, ODD_DIM);
	    return;
	}

	/* Create a test image of the required dimension */
	dim = test_dim_smooth[test_num];
	create(dim);

#ifdef DEBUG
	printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_smooth[bench_index].description);
#endif
	/* Check that the code works */
	run_smooth_benchmark(bench_index, dim);
	if (check_smooth(dim)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_smooth[bench_index].description, dim);
	    return;
	}

	/* Measure CPE */
	{
	    double cpe;
#ifdef OLD
	    double num_cycles;
#endif
	    int tmpdim = dim;
	    void *arglist[4];
	    double dimension = (double) dim;
	    double work = dimension*dimension;
#ifdef DEBUG
	    printf("DEBUG: dimension=%.1f\n",dimension);
	    printf("DEBUG: work=%.1f\n",work);
#endif
	    arglist[0] = (void *) benchmarks_smooth[bench_index].tfunct;
	    arglist[1] = (void *) &tmpdim;
	    arglist[2] = (void *) orig;
	    arglist[3] = (void *) result;
        
	    create(dim);
#ifdef OLD
	    num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist, CLEAR_CACHE); 
	    cpe = num_cycles/work;
#else
            /* work or dimension? */
	    cpe = find_cpe_v((test_funct_v)&func_wrapper, arglist,work);
#endif
	    benchmarks_smooth[bench_index].cpes[test_num] = cpe;
	}
    }

    /* Print results as a table */
    printf("Smooth: Version = %s:\n", description);
    printf("Dim\t");
    for (i = 0; i < DIM_CNT; i++)
	printf("\t%d", test_dim_smooth[i]);
    printf("\tMean\n");
  
    printf("Your CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", benchmarks_smooth[bench_index].cpes[i]);
    }
    printf("\n");

    printf("Baseline CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", smooth_baseline_cpes[i]);
    }
    printf("\n");

    /* Compute speedup */
    {
	double prod, ratio, mean;
	prod = 1.0; /* Geometric mean */
	printf("Speedup\t");
	for (i = 0; i < DIM_CNT; i++) {
	    if (benchmarks_smooth[bench_index].cpes[i] > 0.0) {
		ratio = smooth_baseline_cpes[i]/
		    benchmarks_smooth[bench_index].cpes[i];
	    }
	    else {
		printf("Fatal Error: Non-positive CPE value...\n");
		exit(EXIT_FAILURE);
	    }
	    prod *= ratio;
	    printf("\t%.1f", ratio);
	}
	/* Geometric mean */
	mean = pow(prod, 1.0/(double) DIM_CNT);
	printf("\t%.1f", mean);
	printf("\n\n");
	if (mean > smooth_maxmean) {
	    smooth_maxmean = mean;
	    smooth_maxmean_desc = benchmarks_smooth[bench_index].description;
	}
    }

    return;  
}

void run_line_benchmark(int idx, int dim) 
{
    benchmarks_line[idx].tfunct(dim, orig, result);
}

static short baseline_measured = 0; /* false */

static void measure_baseline() 
{
    int test_num;
  
	
    if (!(line_benchmark_base == -1 || autograder)) { /* not defined */
        printf("\nUsing baseline implementation '%s'\n",  benchmarks_line[line_benchmark_base].description);
    }

    /* Measure baseline CPE */
   for (test_num = 0; test_num < DIM_CNT; test_num++) {
        int dim;

        /* Create a test image of the required dimension */
        dim = test_dim_line[test_num];
        create(dim);

        double cpe;
        double num_cycles;
        int tmpdim = dim;
        void *arglist[4];
        double dimension = (double) dim;
        double work = dimension*dimension;

        if (line_benchmark_base == -1 || autograder) { /* not defined */
            arglist[0] = (void *) (lab_test_func)baseline_line;
            arglist[1] = (void *) &tmpdim;
            arglist[2] = (void *) orig;
            arglist[3] = (void *) result;
        } else {
	    arglist[0] = (void *) benchmarks_line[line_benchmark_base].tfunct;
            arglist[1] = (void *) &tmpdim;
            arglist[2] = (void *) orig;
            arglist[3] = (void *) result;
        }

        create(dim);

        if (autograder || line_benchmark_base == -1) {
	    printf("Running baseline implementation (%dx%d)\n", dim, dim);
        }

        num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist, CLEAR_CACHE);
        cpe = num_cycles/work;
	line_baseline_cpes[test_num] = cpe;

        if (cpe > 1000) {
            printf("line_baseline_cpes[%d]: %.1f (%dx%d)\n",test_num, line_baseline_cpes[test_num],dim,dim);
        }
    }
    /* This ends Measure baseline CPE */
    baseline_measured = 1; /* true */
}

void test_line(int bench_index) 
{
    int i;
    int test_num;
    char *description = benchmarks_line[bench_index].description;

    for(test_num = DIM_CNT- 1; test_num >= 0; test_num--) {
	int dim;

	/* Check correctness for odd (non power of two dimensions */
	create(ODD_DIM);
	run_line_benchmark(bench_index, ODD_DIM);
	if (check_line(ODD_DIM)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_line[bench_index].description, ODD_DIM);
	    return;
	}

	/* Create a test image of the required dimension */
	dim = test_dim_line[test_num];
	create(dim);

#ifdef DEBUG
	printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_line[bench_index].description);
#endif
	/* Check that the code works */
	run_line_benchmark(bench_index, dim);
	if (check_line(dim)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_line[bench_index].description, dim);
	    return;
	}

	/* Measure CPE */
	{
	    double cpe;
	    double num_cycles;
	    int tmpdim = dim;
	    void *arglist[4];
	    double dimension = (double) dim;
	    double work = dimension*dimension;
#ifdef DEBUG
	    printf("\nDEBUG: dimension=%.1f\n",dimension);
#endif
	    arglist[0] = (void *) benchmarks_line[bench_index].tfunct;
	    arglist[1] = (void *) &tmpdim;
	    arglist[2] = (void *) orig;
	    arglist[3] = (void *) result;
        
	    create(dim);

            num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist, CLEAR_CACHE);
            cpe = num_cycles/work;

        if (cpe > 1000) {
            printf("line_cpes[%d]: %.1f (%dx%d)\n",test_num, cpe,dim,dim);
        }

            benchmarks_line[bench_index].cpes[test_num] = cpe;
	}
    }

    /* Print results as a table */
    printf("Line: Version = %s:\n", description);
    printf("Dim\t");
    for (i = 0; i < DIM_CNT; i++)
	printf("\t%d", test_dim_line[i]);
    printf("\tMean\n");
  
    printf("Your CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", benchmarks_line[bench_index].cpes[i]);
    }
    printf("\n");

    if (!baseline_measured)
	measure_baseline();

    printf("Baseline CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", line_baseline_cpes[i]);
    }
    printf("\n");

    /* Compute speedup */
    {
	double prod, ratio, mean;
	prod = 1.0; /* Geometric mean */
	printf("Speedup\t");
	for (i = 0; i < DIM_CNT; i++) {
	    if (benchmarks_line[bench_index].cpes[i] > 0.0) {
		ratio = line_baseline_cpes[i]/
		    benchmarks_line[bench_index].cpes[i];
	    }
	    else {
		printf("Fatal Error: Non-positive CPE value...\n");
		exit(EXIT_FAILURE);
	    }
	    prod *= ratio;
	    printf("\t%.1f", ratio);
	}
	/* Geometric mean */
	mean = pow(prod, 1.0/(double) DIM_CNT);
	printf("\t%.1f", mean);
	printf("\n\n");
	if (mean > line_maxmean) {
	    line_maxmean = mean;
	    line_maxmean_desc = benchmarks_line[bench_index].description;
	}
    }

    return;  
}


void usage(char *progname) 
{
    fprintf(stderr, "Usage: %s [-hqg] [-f <func_file>] [-d <dump_file>]\n", progname);    
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h         Print this message\n");
    fprintf(stderr, "  -q         Quit after dumping (use with -d )\n");
    fprintf(stderr, "  -g         Autograder mode: checks only line, rotate() or smooth()\n");
    fprintf(stderr, "  -f <file>  Get test function names from dump file <file>\n");
    fprintf(stderr, "  -d <file>  Emit a dump file <file> for later use with -f\n");
    exit(EXIT_FAILURE);
}



int main(int argc, char *argv[])
{
    int i;
    int quit_after_dump = 0;
    int skip_teamname_check = 0;
    int seed = 1729;
    char c = '0';
    char *bench_func_file = NULL;
    char *func_dump_file = NULL;

    /* register all the defined functions */
    register_rotate_functions();
    register_smooth_functions();
    register_line_functions();

    /* parse command line args */
    while ((c = getopt(argc, argv, "tgqf:d:s:h")) != -1)
	switch (c) {

	case 't': /* skip team name check (hidden flag) */
	    skip_teamname_check = 1;
	    break;

	case 's': /* seed for random number generator (hidden flag) */
	    seed = atoi(optarg);
	    break;

	case 'g': /* autograder mode (checks only rotate() and smooth()) */
	    autograder = 1;
	    break;

	case 'q':
	    quit_after_dump = 1;
	    break;

	case 'f': /* get names of benchmark functions from this file */
	    bench_func_file = strdup(optarg);
	    break;

	case 'd': /* dump names of benchmark functions to this file */
	    func_dump_file = strdup(optarg);
	    {
		int i;
		FILE *fp = fopen(func_dump_file, "w");	

		if (fp == NULL) {
		    printf("Can't open file %s\n",func_dump_file);
		    exit(-5);
		}

		for(i = 0; i < rotate_benchmark_count; i++) {
		    fprintf(fp, "R:%s\n", benchmarks_rotate[i].description); 
		}
		for(i = 0; i < smooth_benchmark_count; i++) {
		    fprintf(fp, "S:%s\n", benchmarks_smooth[i].description); 
		}
		for(i = 0; i < line_benchmark_count; i++) {
		    fprintf(fp, "L:%s\n", benchmarks_line[i].description); 
		}
		fclose(fp);
	    }
	    break;

	case 'h': /* print help message */
	    usage(argv[0]);

	default: /* unrecognized argument */
	    usage(argv[0]);
	}

    if (quit_after_dump) 
	exit(EXIT_SUCCESS);


    /* Print team info */
    if (!skip_teamname_check) {
	if (strcmp("bovik", team.team) == 0) {
	    printf("%s: Please fill in the team struct in kernel.c.\n", argv[0]);
	    exit(1);
	}
	printf("Teamname: %s\n", team.team);
	printf("Member 1: %s\n", team.name1);
	printf("Email 1: %s\n", team.email1);
	if (*team.name2 || *team.email2) {
	    printf("Member 2: %s\n", team.name2);
	    printf("Email 2: %s\n", team.email2);
	}
	printf("\n");
    }

    srand(seed);

    /* 
     * If we are running in autograder mode, we will only test
     * the line(), rotate() or bench() functions.
     */
    if (autograder) {
	rotate_benchmark_count = 0;
	smooth_benchmark_count = 0;
	line_benchmark_count = 1;

	benchmarks_rotate[0].tfunct = rotate;
	benchmarks_rotate[0].description = "rotate() function";
	benchmarks_rotate[0].valid = 0;

	benchmarks_smooth[0].tfunct = smooth;
	benchmarks_smooth[0].description = "smooth() function";
	benchmarks_smooth[0].valid = 0;

	benchmarks_line[0].tfunct = line;
	benchmarks_line[0].description = "line() function";
	benchmarks_line[0].valid = 1;
    }

    /* 
     * If the user specified a file name using -f, then use
     * the file to determine the versions of rotate and smooth to test
     */
    else if (bench_func_file != NULL) {
	char flag;
	char func_line[256];
	FILE *fp = fopen(bench_func_file, "r");

	if (fp == NULL) {
	    printf("Can't open file %s\n",bench_func_file);
	    exit(-5);
	}
    
	while(func_line == fgets(func_line, 256, fp)) {
	    char *func_name = func_line;
	    char **strptr = &func_name;
	    char *token = strsep(strptr, ":");
	    flag = token[0];
	    func_name = strsep(strptr, "\n");
#ifdef DEBUG
	    printf("Function Description is %s\n",func_name);
#endif

	    if (flag == 'R') {
		for(i=0; i<rotate_benchmark_count; i++) {
		    if (strcmp(benchmarks_rotate[i].description, func_name) == 0)
			benchmarks_rotate[i].valid = 1;
		}
	    }
	    else if (flag == 'S') {
		for(i=0; i<smooth_benchmark_count; i++) {
		    if (strcmp(benchmarks_smooth[i].description, func_name) == 0)
			benchmarks_smooth[i].valid = 1;
		}
	    }      
	    else if (flag == 'L') {
		for(i=0; i<line_benchmark_count; i++) {
		    if (strcmp(benchmarks_line[i].description, func_name) == 0)
			benchmarks_line[i].valid = 1;
		}
	    }      
	}

	fclose(fp);
    }

    /* 
     * If the user didn't specify a dump file using -f, then 
     * test all of the functions
     */
    else { /* set all valid flags to 1 */
	for (i = 0; i < rotate_benchmark_count; i++)
	    benchmarks_rotate[i].valid = 1;
	for (i = 0; i < smooth_benchmark_count; i++)
	    benchmarks_smooth[i].valid = 1;
	for (i = 0; i < line_benchmark_count; i++)
	    benchmarks_line[i].valid = 1;
    }

    /* Set measurement (fcyc) parameters */
    /* new default 512K L2 cache for Pentium III (<<17): set_fcyc_cache_size(1 << 14); // 16 KB cache size */
    /* clear_cache is now a parameter of fcyc (always off): set_fcyc_clear_cache(1); // clear the cache before each measurement */
    /* compensate is now a parameter of fcyc_full (default off): set_fcyc_compensate(1); // try to compensate for timer overhead */
 
    for (i = 0; i < rotate_benchmark_count; i++) {
	if (benchmarks_rotate[i].valid)
	    test_rotate(i);
    
}
    for (i = 0; i < smooth_benchmark_count; i++) {
	if (benchmarks_smooth[i].valid)
	    test_smooth(i);
    }

    for (i = 0; i < line_benchmark_count; i++) {
	if (benchmarks_line[i].valid && (autograder || i != line_benchmark_base))
	    test_line(i);
    }


    if (autograder) {
	printf("\nbestscores:%.1f:%.1f:%.1f\n", rotate_maxmean, smooth_maxmean, line_maxmean);
    }
    else {
	printf("Summary of Your Best Scores:\n");
        if ( rotate_maxmean > 0.0)
	printf("  Rotate: %3.1f (%s)\n", rotate_maxmean, rotate_maxmean_desc);
        if ( smooth_maxmean > 0.0)
	printf("  Smooth: %3.1f (%s)\n", smooth_maxmean, smooth_maxmean_desc);
        if ( line_maxmean > 0.0)
	printf("  Line  : %3.1f (%s)\n", line_maxmean, line_maxmean_desc);
    }

    return 0;
}













