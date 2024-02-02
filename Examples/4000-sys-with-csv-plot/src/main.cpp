
#include "main.h"


int main()
{

 int pid, t;
 unsigned int seed;
 FILE *fp;
 Sys1 p;
 double u[1];
 
#if (DEBUG > 0)
  setvbuf(stdout, (char*) NULL, _IONBF, 0);
  setvbuf(stderr, (char*) NULL, _IONBF, 0);
#endif


  
  /* init random number generator  */
  seed = (unsigned) time(NULL);
  srand(seed);
  pid = getpid();

  /* open file for log */
  fp = fopen("log.csv", "w");
  
  fprintf(fp, "#Start main with pid %ld, ppid %ld. Format: time state[3] output[1] input[1]\n",
	(long) pid, (long) getppid());

  /* init system state */
  p.init();
   

  for (t = 0; t <= HORIZON; t++)
   {
     /* update output */

     p.update_output();

     /* read input */

     u[0] = noise();
     
     //   log to csv
     fprintf(fp, "%d, %lf, %lf, %lf, %lf, %lf\n", t, p.x[0], p.x[1], p.x[2], p.y[0], u[0]);
      
     /* update state */
     p.update_state(u);

   
   }  /* while */


  // close file
  fclose(fp);
  
}  /*  main()  */









