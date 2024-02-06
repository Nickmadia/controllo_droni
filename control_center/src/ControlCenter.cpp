#include "ControlCenter.h"
#include <vector>
// Implementazione dei metodi della classe ControlCenter
ControlCenter::ControlCenter() {/* Costruttore */}

redisReply* ControlCenter::read_stream(redisContext *c, const char *stream_name) {
    // Read from the stream starting from the beginning
    redisReply *reply = (redisReply *)redisCommand(c, "XREAD STREAMS %s 0", stream_name);
    assertReply(c,reply);
    return reply;
}
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

void ControlCenter::await_sync() {
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
    //TODO check wheter the reply contains the sync msg
    freeReplyObject(reply);
    // we could send the time t in the stream or get it calculated from the drones
}

void ControlCenter::tick(int t) {
    switch (currentStatus) {
        case STARTUP:
            //startup
            redisReply *reply = read_stream(this->c, drone_stream);
            assertReplyType(this->c, reply, REDIS_REPLY_ARRAY);
            //redisReply *msgs = reply->element[0];// reading from only one stream so [0]

            // get elements as droneid, 
            int msgs_count = ReadStreamNumMsg(reply, 0);
            for( int i =0 ; i< msgs_count; i++) {
                //should check type remember later + format
                // first field = type, 2nd drone_id
                /* define msg types later as integers
                    wake = 0 ,

                    */
                // field value example should be 0 , 0
                int drone_id = ReadStreamMsgVal(reply, 0, i, 3) ;// reading 3rd element of the msg
                this->addDrone(Drone(drone_id)); // constructor with just the id init drone position at start
            }
            // add drones to the array
            //try read messages, save drones in array 
            
            if (this->drones_array.size() == DRONES_COUNT) {//add DRONES COUNT TO CC attr, get it from command line
                this->currentStatus = READY;
            }
            freeReplyObject(reply);
            break;
        case READY:
            //ready
            // calcola aree
            /*Sub area should be verifiable in <=5 min with just 1 drone
              since a drone flyes at 30km/h, in 5 min it can cover a distance of
              2.5km or 250 points each 10 m apart
              so we need to calculate 300*300(area with points already 20 m apart)/250
              which are roughly 360 sub areas
              so we need atleast 360 drones + drones for when the battery is over*/
            // we ve got to keep in mind that the drones doing the the verify need to also 
            //come back to the starter position before 5 min so area must be of side x | x^2 + sqrt(2x^2) = 2500
            // x = 15.115 so we round down to 15
            // subareas become then 15 x 15 points spaced 20 m apart
            // Total area get divided in 400 sub areas of 225 points(20 m apart) 
            // so there are 20 points on each axis in order to obtain the sub areas

            this->
            // assegna ogni area ad un drone
            //go to running
            break;

        case RUNNING:
            //running
            //read messages
            /*if drone deve tornare : ()
                fallo tornare
                manda un altro*/ 
            // wait drone statuses for logs bloccante 
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
