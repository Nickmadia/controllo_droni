#include "main.h"

int test1(redisContext *context)
{
    redisReply *reply;
    
    if (!context) {
        fprintf(stderr, "Error:  Can't connect to Redis\n");
        return -1;
    }

    reply = (redisReply *) redisCommand(context, "PING");
    if (!reply || context->err) {
        fprintf(stderr, "Error:  Can't send command to Redis\n");
        return -1;
    }
    printf("redis: %s\n", reply->str);
    freeReplyObject(reply);

    
    reply = (redisReply *) redisCommand(context, "SET %s %s", "foo", "bar");
    if (!reply || context->err) {
        fprintf(stderr, "Error:  Can't send command to Redis\n");
        return -1;
    }
   printf("redis: %s\n", reply->str);
   freeReplyObject(reply);

    reply = (redisReply *) redisCommand(context, "GET foo");
    if (!reply || context->err || reply->type != REDIS_REPLY_STRING) {
        fprintf(stderr, "Error:  Can't send command to Redis\n");
        return -1;
    }
    printf("redis: %s\n", reply->str);
    printf("foo = %s\n", reply->str);
    freeReplyObject(reply);

     
    return 0;
}


