#include "ControlCenter.h"
#include <vector>
// Implementazione dei metodi della classe ControlCenter
ControlCenter::ControlCenter() {/* Costruttore */}

void ControlCenter::addDrone(Drone drone) {
    // Aggiunge un drone al centro di controllo
    drones.push_back(drone);
}

void ControlCenter::init() {
    //setup conn to redis
    pid = getpid();

    const char *hostname = "127.0.0.1";
    int port = 6379;
    c = redisConnect(hostname,port);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    //set up sync streams
    redisReply *reply;
    reply = (redisReply *)redisCommand(c, "EXISTS %s", sync_stream);
    assertReplyType(c, reply, REDIS_REPLY_INTEGER);
    if ( !reply->integer ) {
        initStreams(c, sync_stream);
    }
    freeReplyObject(reply);

    //set up drone stream
    reply = (redisReply *)redisCommand(c, "EXISTS %s", drone_stream);
    assertReplyType(c, reply, REDIS_REPLY_INTEGER);
    if (! reply-> integer) {
        initStreams(c , drone_stream);
    }
    freeReplyObject(reply);


}

void ControlCenter::sync() {
    redisReply *reply;

    // Send a synchronization message to the Redis stream
    const char *message_id = "*"; // Send to the latest message in the stream
    const char *message = "Sync";
    reply = (redisReply *)redisCommand(c, "XADD %s %s message %s", sync_stream, message_id, message);
    assertReplyType(c, reply, REDIS_REPLY_STRING);
    //if needed log reply later
    freeReplyObject(reply);

    // Block to receive synchronization messages from the Redis stream
    reply = (redisReply *)redisCommand(c, "XREAD BLOCK %s STREAMS %s $", block_time, sync_stream);
    assertReply(c, reply);

    // if needed dump reply or read it, but shouldnt be necessary atm
    freeReplyObject(reply);
    // we could send the time t in the stream or get it calculated from the drones
}

void ControlCenter::tick(int t) {
    switch (currentStatus) {
        case startup:
            //startup
            //try read messages, save drones in array 
            // if len(drones_array) == total drones go to ready
            break;
        case ready:
            //ready
            // calcola aree
            // assegna ogni area ad un drone
            //go to running
            break;

        case running:
            //running
            //read messages
            /*if drone deve tornare : (autonomia <= distanza control_center x2)
                fallo tornare
                manda un altro*/ 
            // log stuff 
            break;
        default:
            //no status 
            break;
    }
}
void ControlCenter::sendInstructions() {
    // Implementazione dell'invio di istruzioni ai droni
    // ...
}

void ControlCenter::updateMonitoringData() {
    // Implementazione dell'aggiornamento dei dati di monitoraggio
    // ...
}

std::vector<Drone>& ControlCenter::getDrones() {
    return drones;
}

// Altri metodi della classe ControlCenter
// ...
