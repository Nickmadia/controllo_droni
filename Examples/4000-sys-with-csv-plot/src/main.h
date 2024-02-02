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

#include "sys1.h"


#define DEBUG 1000

#define HORIZON 1000  // TICKS

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define random_double_0to1()  (((double) rand())/((double) RAND_MAX))

double noise();

using namespace std;

#endif
