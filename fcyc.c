/* Compute time used by function f */
#include <stdlib.h>
#include <sys/times.h>
#include <stdio.h>
#include <string.h>

#include "clock.h"
#include "fcyc.h"

static double *values = NULL;
int samplecount = 0;

#define KEEP_VALS 1
#define KEEP_SAMPLES 1

#if KEEP_SAMPLES
double *samples = NULL;
#endif

/* Start new sampling process */
static void init_sampler(int k, int maxsamples)
{
  if (values)
    free(values);
  values = calloc(k, sizeof(double));
#if KEEP_SAMPLES
  if (samples)
    free(samples);
  /* Allocate extra for wraparound analysis */
  samples = calloc(maxsamples+k, sizeof(double));
#endif
  samplecount = 0;
}

/* Add new sample.  */
void add_sample(double val, int k)
{
  int pos = 0;
  if (samplecount < k) {
    pos = samplecount;
    values[pos] = val;
  } else if (val < values[k-1]) {
    pos = k-1;
    values[pos] = val;
  }
#if KEEP_SAMPLES
  samples[samplecount] = val;
#endif
  samplecount++;
  /* Insertion sort */
  while (pos > 0 && values[pos-1] > values[pos]) {
    double temp = values[pos-1];
    values[pos-1] = values[pos];
    values[pos] = temp;
    pos--;
  }
}

/* Get current minimum */
double get_min()
{
  return values[0];
}

/* What is relative error for kth smallest sample */
double err(int k)
{
  if (samplecount < k)
    return 1000.0;
  return (values[k-1] - values[0])/values[0];
}

/* Have k minimum measurements converged within epsilon? */
int has_converged(int k_arg, double epsilon_arg, int maxsamples)
{
  if ((samplecount >= k_arg) &&
      ((1 + epsilon_arg)*values[0] >= values[k_arg-1]))
    return samplecount;
  if ((samplecount >= maxsamples))
    return -1;
  return 0;
}

/* estimate cache_size */
static int cache_size = 0;
static int block_size = 0;

/* Get cache_size from /etc/proc */
#define MAXBUF 512
int get_cache_size(int verbose) {
    static char buf[MAXBUF];
    static char units[MAXBUF];
    FILE *fp = fopen("/proc/cpuinfo", "r");
    cache_size = 0;

    if (!fp) {
        fprintf(stderr, "Can't open /proc/cpuinfo to get clock information\n");
        cache_size = 1024; /* Kb */
        return cache_size;
    }
    while (fgets(buf, MAXBUF, fp)) {
        if (strstr(buf, "cache size")) {
            sscanf(buf, "cache size\t: %d %s", &cache_size,units);
            break;
        }
    }
    if (cache_size == 0 || strncmp(units,"KB",2) != 0) {
        fprintf(stderr, "Can't open /proc/cpuinfo to get cache information\n");
        cache_size = 1024; /* Kb */
        return cache_size;
    }
    if (verbose) {
        printf("Processor Cache Size ~= %d Kb\n", cache_size);
    }
    return cache_size;
}

/* Get block_size with cpuid (information in /proc/cpuid not relevant) */

#if __GNUC__ > 2
#define access_cpuid(n,w,x,y,z) \
  asm("cpuid" : "=a" (w), "=b" (x), "=c" (y), "=d" (z) : "a" (n))
#else
#define access_cpuid(n,w,x,y,z) \
  asm("cpuid" : "=eax" (w), "=ebx" (x), "=ecx" (y), "=edx" (z) : "0" (n))
#endif

#define extended    0x80000000UL        /* start of extended cpuid range     */
#define level2_info 0x80000006UL        /* request L2 cache info directly    */


int get_block_size(int verbose) {

  ulong register_a, register_b, register_c, register_d;
  ulong level, value;

  block_size = 0;

  /* get extended level */
  access_cpuid(extended, register_a, register_b, register_c, register_d);

  if(verbose > 1 &&  register_a >= extended) {
    printf("%08lx : %08lx %08lx %08lx %08lx\n",
      extended, register_a, register_b, register_c, register_d);
  }

  level = register_a;

  if (level >= level2_info) {
      access_cpuid(level2_info, register_a, register_b, register_c, register_d);

      value = (register_c >> 0) & 0xff;
      block_size = (int )value;

      if(verbose > 2) {
          printf("%08lx : %08lx %08lx %08lx %08lx\n",
                level2_info, register_a, register_b, register_c, register_d);
          printf("\n");

          if (value != 0x00) {
              printf("unified L2 cache line size in bytes   : 0x%02lxh (%d bytes)\n", value, block_size);
              printf("\n");
          }
      }
  }

  if (block_size == 0 ) {
        fprintf(stderr, "Can't determine block size with cpuid \n");
        block_size = 32; /* Kb */
        return block_size;
    }
    if (verbose) {
        printf("Processor Block Size ~= %d Kb\n", block_size);
    }
    return block_size;
}

/* Code to clear cache */
/* Pentium III has 512K L2 cache, which is 128K ints */
#define MANYINTS512 (1 << 17)
/* Cache block size is 32 bytes */
static int stuff512[MANYINTS512];

/* Intel Core 2 Duo has 1024K L2 cache, which is 256K ints */
#define MANYINTS1024 (1 << 18)
/* Cache block size is 64 bytes */
static int stuff1024[MANYINTS1024];
static int sink;

static void clear()
{
  int x = sink;
  int c = cache_size;
  int b = block_size;
  int stride;
  int i;

  if (c == 0)
	get_cache_size(1);

  if (b == 0)
	get_block_size(1);

  stride = block_size / sizeof(int);

  if (c == 512 && b == 32) {
        /* printf("Clearing cache with %d ints and stride %d\n", MANYINTS512, stride); */
  	for (i = 0; i < MANYINTS512; i += stride)
    		x += stuff512[i];
  }
  if (c == 1024 && b == 64) {
        /* printf("Clearing cache with %d ints and stride %d\n", MANYINTS1024, stride); */
  	for (i = 0; i < MANYINTS1024; i += stride)
    		x += stuff1024[i];
  }
  sink = x;
}

double fcyc_full(test_funct f, int param, int clear_cache,
		 int k, double epsilon, int maxsamples, int compensate) 
{
  double result;
  init_sampler(k, maxsamples);
  if (compensate) {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_comp_counter();
      f(param);
      cyc = get_comp_counter();
      add_sample(cyc, k);
    } while (!has_converged(k, epsilon, maxsamples) && samplecount < maxsamples);
  } else {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_counter();
      f(param);
      cyc = get_counter();
      add_sample(cyc, k);
    } while (!has_converged(k, epsilon, maxsamples) && samplecount < maxsamples);
  }
#ifdef DEBUG
  {
    int i;
    printf(" %d smallest values: [", k);
    for (i = 0; i < k; i++)
      printf("%.0f%s", values[i], i==k-1 ? "]\n" : ", ");
  }
#endif
  result = values[0];
#if !KEEP_VALS
  free(values); 
  values = NULL;
#endif
  return result;  
}

double fcyc(test_funct f, int param, int clear_cache)
{
  return fcyc_full(f, param, clear_cache, 3, 0.01, 20, 0);
}


/******************* Version that uses argument list *************/

double fcyc_full_v(test_funct_v f, void* params[], int clear_cache,
		 int k, double epsilon, int maxsamples, int compensate) 
{
  double result;
  init_sampler(k, maxsamples);
  if (compensate) {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_comp_counter();
      f(params);
      cyc = get_comp_counter();
      add_sample(cyc, k);
    } while (!has_converged(k, epsilon, maxsamples) && samplecount < maxsamples);
  } else {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_counter();
      f(params);
      cyc = get_counter();
      add_sample(cyc, k);
    } while (!has_converged(k, epsilon, maxsamples) && samplecount < maxsamples);
  }
#ifdef DEBUG
  {
    int i;
    printf(" %d smallest values: [", k);
    for (i = 0; i < k; i++)
      printf("%.0f%s", values[i], i==k-1 ? "]\n" : ", ");
  }
#endif
  result = values[0];
#if !KEEP_VALS
  free(values); 
  values = NULL;
#endif
  return result;  
}

double fcyc_v(test_funct_v f, void* params[], int clear_cache)
{
  return fcyc_full_v(f, params, clear_cache, 3, 0.01, 20, 0);
}


/******************* Version that uses gettimeofday *************/

static double Mhz = 0.0;

#include <sys/time.h>

static struct timeval tstart;

/* Record current time */
void start_counter_tod()
{
  if (Mhz == 0)
    Mhz = mhz_full(0, 10);
  gettimeofday(&tstart, NULL);
}

/* Get number of seconds since last call to start_timer */
double get_counter_tod()
{
  struct timeval tfinish;
  long sec, usec;
  gettimeofday(&tfinish, NULL);
  sec = tfinish.tv_sec - tstart.tv_sec;
  usec = tfinish.tv_usec - tstart.tv_usec;
  return (1e6 * sec + usec)*Mhz;
}

/** Special counters that compensate for timer interrupt overhead */

static double cyc_per_tick = 0.0;

#define NEVENT 100
#define THRESHOLD 1000
#define RECORDTHRESH 3000

/* Attempt to see how much time is used by timer interrupt */
static void callibrate(int verbose)
{
  double oldt;
  struct tms t;
  clock_t oldc;
  int e = 0;
  times(&t);
  oldc = t.tms_utime;
  start_counter_tod();
  oldt = get_counter_tod();
  while (e <NEVENT) {
    double newt = get_counter_tod();
    if (newt-oldt >= THRESHOLD) {
      clock_t newc;
      times(&t);
      newc = t.tms_utime;
      if (newc > oldc) {
	double cpt = (newt-oldt)/(newc-oldc);
	if ((cyc_per_tick == 0.0 || cyc_per_tick > cpt) && cpt > RECORDTHRESH)
	  cyc_per_tick = cpt;
	/*
	if (verbose)
	  printf("Saw event lasting %.0f cycles and %d ticks.  Ratio = %f\n",
		 newt-oldt, (int) (newc-oldc), cpt);
	*/
	e++;
	oldc = newc;
      }
      oldt = newt;
    }
  }
  if (verbose)
    printf("Setting cyc_per_tick to %f\n", cyc_per_tick);
}

static clock_t start_tick = 0;

void start_comp_counter_tod() {
  struct tms t;
  if (cyc_per_tick == 0.0)
    callibrate(1);
  times(&t);
  start_tick = t.tms_utime;
  start_counter_tod();
}

double get_comp_counter_tod() {
  double time = get_counter_tod();
  double ctime;
  struct tms t;
  clock_t ticks;
  times(&t);
  ticks = t.tms_utime - start_tick;
  ctime = time - ticks*cyc_per_tick;
  /*
  printf("Measured %.0f cycles.  Ticks = %d.  Corrected %.0f cycles\n",
	 time, (int) ticks, ctime);
  */
  return ctime;
}


double fcyc_full_tod(test_funct f, int param, int clear_cache,
		 int k, double epsilon, int maxsamples, int compensate) 
{
  double result;
  init_sampler(k, maxsamples);
  if (compensate) {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_comp_counter_tod();
      f(param);
      cyc = get_comp_counter_tod();
      add_sample(cyc, k);
    } while (!has_converged(k, epsilon, maxsamples) && samplecount < maxsamples);
  } else {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_counter_tod();
      f(param);
      cyc = get_counter_tod();
      add_sample(cyc, k);
    } while (!has_converged(k, epsilon, maxsamples) && samplecount < maxsamples);
  }
#ifdef DEBUG
  {
    int i;
    printf(" %d smallest values: [", k);
    for (i = 0; i < k; i++)
      printf("%.0f%s", values[i], i==k-1 ? "]\n" : ", ");
  }
#endif
  result = values[0];
#if !KEEP_VALS
  free(values); 
  values = NULL;
#endif
  return result;  
}

double fcyc_tod(test_funct f, int param, int clear_cache)
{
  return fcyc_full_tod(f, param, clear_cache, 3, 0.01, 20, 0);
}






