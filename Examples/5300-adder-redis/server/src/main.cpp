
#include "main.h"

// cc -Wall -g -ggdb -o streams streams.c -lhiredis
// Usage: ./streams <add count> <read count> [block time, default: 1]

#define DEBUG 1000

#define READ_STREAM "stream2"
#define WRITE_STREAM "stream1"

using namespace std;


int main() {
  
    redisContext *c2r;
    redisReply *reply;
   int read_counter = 0;
    int send_counter = 0;
    int block = 1000000000;
    int pid;
    unsigned seed;
    char username[100];
    char key[100];
    char value[100];
    char streamname[100];
    char msgid[100];
    char fval[100];
    int i, k, h, sum;
    
    /*  prg  */

#if (DEBUG > 0)
  setvbuf(stdout, (char*) NULL, _IONBF, 0);
  setvbuf(stderr, (char*) NULL, _IONBF, 0);
#endif

  seed = (unsigned) time(NULL);
  srand(seed);

  sprintf(username, "%u", rand());
 
  pid = getpid();
    
  printf("main(): pid %d: user %s: connecting to redis ...\n", pid, username);

 c2r = redisConnect("localhost", 6379);
    
 printf("main(): pid %d: user %s: connected to redis\n", pid, username);

   // delete stream if it exists
 reply = RedisCommand(c2r, "DEL %s", READ_STREAM);
 assertReply(c2r, reply);
 dumpReply(reply, 0);

 reply = RedisCommand(c2r, "DEL %s", WRITE_STREAM);
 assertReply(c2r, reply);
 dumpReply(reply, 0);

 /* Create streams/groups */
    initStreams(c2r, READ_STREAM);
    initStreams(c2r, WRITE_STREAM);


    
 while (1)
   {
     //  read
     read_counter++;
     reply = RedisCommand(c2r, "XREADGROUP GROUP diameter %s BLOCK %d COUNT 2 NOACK STREAMS %s >", 
			  username, block, READ_STREAM);

     printf("main(): pid %d: user %s: Read msg %d from stream %s\n", pid, username, read_counter, READ_STREAM);

     assertReply(c2r, reply);
     dumpReply(reply, 0);

     // compute sum of args
     sum = 0;
for (k=0; k < ReadNumStreams(reply); k++)
   {
     ReadStreamName(reply, streamname, k);
   for (i=0; i < ReadStreamNumMsg(reply, k); i++)
     {
       ReadStreamNumMsgID(reply, k, i, msgid);
 
       printf("main(): pid %d: user %s: stream %s, streamnum %d, msg %d, msgid %s with %d values\n",
	      pid, username, streamname,
	      k, i, msgid,
	      ReadStreamMsgNumVal(reply, k, i));
	      for (h = 1; h < ReadStreamMsgNumVal(reply, k, i); h = h + 2)
		{
		  ReadStreamMsgVal(reply, k, i, h, fval);
		  printf("main(): pid %d: user %s: streamnum %d, msg %d, msgid %s value %d = %s\n", pid, username, k, i, msgid, h, fval);

		  sum = sum + atoi(fval);
		}
	printf("main(): pid %d: user %s: sum = %d\n", pid, username, sum);
	      
     }
   }

     
     freeReplyObject(reply);

     // send result
     send_counter++;
     sprintf(key, "sum");
     sprintf(value, "%d", sum);
     
     reply = RedisCommand(c2r, "XADD %s * %s %s", WRITE_STREAM, key, value);
     assertReplyType(c2r, reply, REDIS_REPLY_STRING);
     printf("main(): pid =%d: stream %s: Added %s -> %s (id: %s)\n", pid, WRITE_STREAM, key, value, reply->str);
     freeReplyObject(reply);


     /* sleep   */
     micro_sleep(5000000);
   }  // while ()
 


    redisFree(c2r);
}
