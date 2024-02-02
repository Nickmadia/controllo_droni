
#include "main.h"


int main()
{
Con2DB db1("localhost", "5432", "trafficlight3000", "47002", "logdb_trafficlight3000");

 PGresult *res;
 
  char buf[200];
  
 int pid;

#if (DEBUG > 0)
  setvbuf(stdout, (char*) NULL, _IONBF, 0);
  setvbuf(stderr, (char*) NULL, _IONBF, 0);
#endif


  traffic_light_type x, y;
  int t = 0;
  
  /* init random number generator  */
  srand((unsigned) time(NULL));


  pid = getpid();

 printf("Start main with pid %ld, ppid %ld \n",
	(long) pid, (long) getppid());

 printf(
"Format: time step, global time in second, elapsed time in sec, present time in nanoseconds, variable, value\n");

 init_logdb(db1, pid);

 
   
 /* init time */
 init_time();
   
/* init traffic light state */
 x = init();
 nanos = get_nanos();

 while (t <= HORIZON)
   {
     /* output */

     //   nanos_day = get_day_nanos(buf);

     nanos_day = nanos2day(buf, nanos);

      
    printf("%d, %lf, %lf, %ld, %d, %s, %ld\n", t, global_time_sec, timeadvance, nanos, x, buf, nanos_day);

    log2db(db1, pid, nanos, x);
  
     /* update state */
     y = next(x);

     assert((y == x) || (y == (x + 1)%3));

     x = y;
     
     /* update time */
     t++;

     /* sleep   */
     micro_sleep(500);

     update_time(); // update nanos, global_time_sec, timeadvance,
     
   }  /* while */

 
 log2stdout(db1, pid);
 
}  /*  main()  */









