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
#include "../../con2db/pgsql.h"

#define DEBUG 1000

#define HORIZON 50  // TICKS

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define random_double_0to1()  (((double) rand())/((double) RAND_MAX))



typedef enum {GREEN, ORANGE, RED} traffic_light_type;

traffic_light_type init();
traffic_light_type next(traffic_light_type x);

int msleep(long msec);
int micro_sleep(long usec);
int long get_nanos(void);
void init_time();
void update_time();

long int get_day_nanos(char* buf);
void int2color(char *buf, traffic_light_type x);
void init_logdb(Con2DB db1);
void log2db(Con2DB db1, int pid, char *datebuf, long int nanos_day, traffic_light_type x);




using namespace std;

#endif
