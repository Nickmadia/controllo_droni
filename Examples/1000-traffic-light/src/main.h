#ifndef main_h
#define main_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stddef.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#include <gsl/gsl_fit.h>
#include <cassert>
#include <cerrno>

#include "global.h"

#define HORIZON 20  // TICKS

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define myrandom()  (((double) rand())/((double) RAND_MAX))

#define DEBUG 1000

typedef enum {GREEN, ORANGE, RED} traffic_light_type;

traffic_light_type init();
traffic_light_type next(traffic_light_type x);

int msleep(long msec);
int micro_sleep(long usec);
int long get_nanos(void);
void init_time();
void update_time();

long int get_day_nanos(char* buf);

using namespace std;

#endif
