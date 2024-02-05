
#include "main.h"

/* buy stock  */

void log2db(Con2DB db1, int pid, char *datebuf, long int nanos_day, traffic_light_type x)
{

 PGresult *res;
 int rows, k;
 char trcolor[20];
 int vid = 0;
 
 /*  init  */
 int2color(trcolor, x);

   sprintf(sqlcmd, "SELECT vid FROM TimeVar where (varname = \'x\')");
   res = db1.ExecSQLtuples(sqlcmd);
   vid = atoi(PQgetvalue(res, 0, PQfnumber(res, "vid")));
   PQclear(res);

 #if (DEBUG > 0)
   fprintf(stderr, "log2db(): vid = %d\n", vid);
 #endif

   
      sprintf(sqlcmd, "BEGIN"); 
      res = db1.ExecSQLcmd(sqlcmd);
      PQclear(res);

      
  sprintf(sqlcmd,
"INSERT INTO LogTable VALUES (\'%s\', %ld, %d, %d, %d, \'%s\') ON CONFLICT DO NOTHING",
	  datebuf,
	  nanos_day,
	  pid,
	  vid,
	  x,
	  trcolor
	  );

  res = db1.ExecSQLcmd(sqlcmd);
     PQclear(res);
     
 sprintf(sqlcmd, "COMMIT"); 
      res = db1.ExecSQLcmd(sqlcmd);
      PQclear(res);

      
#if (DEBUG > 0)

      //    fprintf(stderr, "log2db(): check insertion\n");
    
  sprintf(sqlcmd, "SELECT * FROM LogTable");

    res = db1.ExecSQLtuples(sqlcmd);
    rows = PQntuples(res);
      
    fprintf(stderr, "log2db(): inserted in LogTable (\'%s\', %ld, %d, %d, %d, \'%s\')\n",
	    PQgetvalue(res, 0, PQfnumber(res, "timevalue")),
	    strtol(PQgetvalue(res, 0, PQfnumber(res, "nanosec")), NULL, 10),
	    atoi(PQgetvalue(res, 0, PQfnumber(res, "pid"))),
	    atoi(PQgetvalue(res, 0, PQfnumber(res, "vid"))),
	    atoi(PQgetvalue(res, 0, PQfnumber(res, "varvalue"))),
	    PQgetvalue(res, 0, PQfnumber(res, "loginfo"))
	    );
    PQclear(res);
#endif

   
 

      
 }  /*   log2db()  */
 