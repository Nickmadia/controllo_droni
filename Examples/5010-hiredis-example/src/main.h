#ifndef main_h
#define main_h

#include <stdio.h>
#include <stdlib.h>

// include hiredis
extern "C" {
#include <hiredis/hiredis.h>
}

// #include "global.h"
// #include "../../con2db/pgsql.h"

#include "macro.h"

void assertReplyType(redisContext *c, redisReply *r, int type);

void assertReply(redisContext *c, redisReply *r);

void dumpReply(redisReply *r, int indent);

void initStreams(redisContext *c, const char *stream);

int test1(redisContext *context);

using namespace std;

#endif
