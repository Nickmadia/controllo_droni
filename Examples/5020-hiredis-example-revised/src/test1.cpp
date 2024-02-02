#include "main.h"

int test1(redisContext *context)
{
    redisReply *reply;

    //    context = redisConnect("127.0.0.1", 6379);
    
    if (!context) {
        fprintf(stderr, "Error:  Can't connect to Redis\n");
        return -1;
    }

     printf("test1(): BEGIN\n");

     printf("test1(): sending command: \"PING\"\n");

	 reply = (redisReply *) redisCommand(context, "PING");
    if (!reply || context->err) {
        fprintf(stderr, "Error:  Can't send command to Redis\n");
        return -1;
    }
    printf("redis: %s\n", reply->str);
    freeReplyObject(reply);

    printf("test1(): done sending command: \"PING\"\n");
    
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

 
    printf("test1(): END\n");

    return 0;
}


