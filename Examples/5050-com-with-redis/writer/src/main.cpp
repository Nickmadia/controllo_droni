
#include "main.h"

// cc -Wall -g -ggdb -o streams streams.c -lhiredis
// Usage: ./streams <add count> <read count> [block time, default: 1]

#define DEBUG 1000

#define STREAM_NAME "example-stream"

using namespace std;


int main() {
  
    redisContext *c2r;
    redisReply *reply;
    int add = 3;
    int read = 5;
    // int block = 2;
    int pid;
    
    /*  prg  */

#if (DEBUG > 0)
  setvbuf(stdout, (char*) NULL, _IONBF, 0);
  setvbuf(stderr, (char*) NULL, _IONBF, 0);
#endif


  pid = getpid();
    
  printf("main(): pid %d: connecting to redis ...\n", pid);

 c2r = redisConnect("localhost", 6379);
    
 printf("main(): pid %d: connected to redis\n", pid);

    test1(c2r);
      
 if (add < 0 || read < 0)
        dbg_abort("Neither read or add can be negative (read: %d, add: %d)", read, add);



    
    /* Create streams/groups */
    initStreams(c2r, STREAM_NAME);

    for (int i = 0; i < add; i++) {
        reply = RedisCommand(c2r, "XADD %s * foo mem:%d", STREAM_NAME, i);
        assertReplyType(c2r, reply, REDIS_REPLY_STRING);
        printf("main(): pid =%d: Added foo -> mem:%d (id: %s)\n", pid, i, reply->str);
        freeReplyObject(reply);
    }

#if 0
    for (int i = 0; i < read; i++) {
      printf("mina(): pid %d: [%d] Sending XREADGROUP (stream: %s, BLOCK: %d)\n", pid, i, STREAM_NAME, block);
        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                         block, STREAM_NAME);

        assertReply(c2r, reply);

        dumpReply(reply, 0);
        freeReplyObject(reply);
    }

#endif

    redisFree(c2r);
}
