
#include "main.h"

// cc -Wall -g -ggdb -o streams streams.c -lhiredis
// Usage: ./streams <add count> <read count> [block time, default: 1]

#define DEBUG 1000

#define STREAM_NAME "example-stream"


int main() {
  
    redisContext *c2r;
    redisReply *reply;
    int add = 3;
    int read = 5;
    int block = 2;
    
    /*  prg  */

    c2r = redisConnect("localhost", 6379);
    
    test1(c2r);
      
    if (add < 0 || read < 0)
        dbg_abort("Neither read or add can be negative (read: %d, add: %d)", read, add);



    
    /* Create streams/groups */
    initStreams(c2r, STREAM_NAME);

    for (int i = 0; i < add; i++) {
        reply = RedisCommand(c2r, "XADD %s * foo mem:%d", STREAM_NAME, i);
        assertReplyType(c2r, reply, REDIS_REPLY_STRING);
        printf("Added foo -> mem:%d (id: %s)\n", i, reply->str);
        freeReplyObject(reply);
    }

    for (int i = 0; i < read; i++) {
        printf("[%d] Sending XREADGROUP (stream: %s, BLOCK: %d)\n", i, STREAM_NAME, block);
        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                         block, STREAM_NAME);

        assertReply(c2r, reply);

        dumpReply(reply, 0);
        freeReplyObject(reply);
    }

    redisFree(c2r);
}
