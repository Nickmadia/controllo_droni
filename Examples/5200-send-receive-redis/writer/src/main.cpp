
#include "main.h"

// cc -Wall -g -ggdb -o streams streams.c -lhiredis
// Usage: ./streams <add count> <read count> [block time, default: 1]

#define DEBUG 1000

#define READ_STREAM "stream1"
#define WRITE_STREAM "stream2"

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

      /* Create streams/groups */
    initStreams(c2r, READ_STREAM);
    initStreams(c2r, WRITE_STREAM);


    
 while (1)
   {
     // send
     send_counter++;
     sprintf(key, "mykey:%d", send_counter);
     sprintf(value, "myvalue:%d", send_counter);
     
     reply = RedisCommand(c2r, "XADD %s * %s %s", WRITE_STREAM, key, value);
     assertReplyType(c2r, reply, REDIS_REPLY_STRING);
     printf("main(): pid =%d: stream %s: Added %s -> %s (id: %s)\n", pid, WRITE_STREAM, key, value, reply->str);
     freeReplyObject(reply);

     
     //  read
     read_counter++;
     printf("main(): pid %d: user %s: Trying to read msg %d from stream %s\n", pid, username, read_counter, READ_STREAM);

     reply = RedisCommand(c2r, "XREADGROUP GROUP diameter %s BLOCK %d COUNT 1 NOACK STREAMS %s >", 
			  username, block, READ_STREAM);

     assertReply(c2r, reply);
     dumpReply(reply, 0);
     freeReplyObject(reply);

     /* sleep   */
     micro_sleep(1000000);
   }  // while ()
 


    redisFree(c2r);
}
