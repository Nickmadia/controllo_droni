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
    this->pid = getpid();

    const char *hostname = "127.0.0.1";
    int port = 6379;
    this->c = redisConnect(hostname,port);
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

    //set up stream for receiving from drone
    reply = (redisReply *)redisCommand(c, "EXISTS %s", drone_stream);
    assertReplyType(c, reply, REDIS_REPLY_INTEGER);
    if (! reply-> integer) {
        initStreams(c , drone_stream);
    }
    freeReplyObject(reply);
    

    //set up streams to send to a single drone create one stream for each drone
    for (int i = 0; i< DRONES_COUNT; i++){
    char * str[8];
    itoa(i,str, 10)
    reply = (redisReply *)redisCommand(c, "EXISTS %s", str );
    assertReplyType(c, reply, REDIS_REPLY_INTEGER);
    if (! reply-> integer) {
        initStreams(c , str);
    }
    freeReplyObject(reply);
    }


}

void ControlCenter::await_sync() {
    redisReply *reply;

    // Send a synchronization message to the Redis stream
    const char *message_id = "*"; // Send to the latest message in the stream
    const char *message = "Sync";
    reply = (redisReply *)redisCommand(c, "XADD %s %s type %s", sync_stream, message_id, message);
    assertReplyType(c, reply, REDIS_REPLY_STATUS);
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
            char value[8];
            for( int i =0 ; i< msgs_count; i++) {
                //should check type remember later + format
                // first field = type, 2nd drone_id
                /* define msg types later as integers
                    wake = 0 ,

                    */
                // field value example should be 0 , 0
                ReadStreamMsgVal(reply, 0, i, 3, value) ;// reading 3rd element of the msg
                int drone_id = atoi(value);
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
              2.5km or 125 points each 20 m apart
              so we need to calculate 300*300(area with points already 20 m apart)/125
              which are roughly 720 sub areas
              so we need atleast 720 drones + drones for when the battery is over
              
              estimated drones 720 * time of charge / autonomy = 4k drones*/
            // we ve got to keep in mind that the drones which are doing the verify need also to
            //come back to the starter position within the area before the 5 min mark
            // so the subarea must be a square of side x | x^2 + sqrt(2x^2) = 125 
            // x = 10.495 so we round down to 10
            // subareas become then 10 x 10 points spaced 20 m apart
            // Total area gets divided in 900 sub areas of 100 points(20 m apart) 
            // so there are 30 points on each axis in order to obtain the sub areas

            /*   subarea 10x10 points 
                +------+------+------+------+------+(300x300 * (points) 20 m apart)
                |***** |***** |***** |***** |***** |
                |***** |***** |***** |***** |***** |
                |***** |***** |***** |***** |***** |
                +------+------+------+------+------+
                |***** |***** |***** |***** |***** |
                |***** |***** |***** |***** |***** |
                |***** |***** |***** |***** |***** |
                +------+------+------+------+------+
                |***** |***** |***** |***** |***** |
                |***** |***** |***** |***** |***** |
                |***** |***** |***** |***** |***** |
                +------+------+------+------+------+
                (30x30 subareas)
*/
            //every drone gets the top-left coordinate, starting point(sp), and starting direction(sdx)  as [1,-1] only or rigth left  
            // 

            //we identify each subarea with the top-left coordinate and the side of length 10 points

            //we consider at the start the full swarm to be charged and ready
            for (int i=0; i<SUB_AREAS_H; i++) {
                for(int j =0; j< SUB_AREAS_W; j++) {
                    //send message to drone this->drones[i*subareasy + subareax] with coord (i,j), sp and sd

                    char *message_id = "*"; // Send to the latest message in the stream
                    // jobmsg  e.g. y 80 x 50 spy 90 spx 60 sdx -1 (left) 
                    // also add all of this in a job struct to be associated with the drone obj 

                    char stream_n[8]; //stream number associated with the drone
                    itoa(i*SUB_AREAS_Y + j,stream_n, 10); // convert int to str

                    Job job = create_job(i,j); // returns a job struct using the subarea coord.
                    reply = (redisReply *)redisCommand(c, "XADD %s %s %s", stream_n, message_id, get_job_msg(job));//job.msg() returns the formated job in a string-type in order to send it via stream
                    assertReplyType(c, reply, REDIS_REPLY_STATUS);

                    freeReplyObject(reply);
                    // change drone status to running
                    this->drones[i*SUB_AREAS_H + j].job = job;
                    this->drones[i*SUB_AREAS_H + j].status = FLYING;
                }
            }
            // assegna ogni area ad un drone

            //go to running
            this->currentStatus = RUNNING;
            break;

        case RUNNING:
            //running
            //read messages

            // once the cc is up and running, it only recevices msgs and responds accordingly to each one
            
            //read all msgs in the stream at time t
            redisReply *reply = read_stream(this->c, drone_stream);
            assertReplyType(this->c, reply, REDIS_REPLY_ARRAY);

            int msgs_count = ReadStreamNumMsg(reply, 0);
            char msg_type[50];
            for( int i =0 ; i< msgs_count; i++) {
                ReadStreamMsgVal(reply, 0, i, 1, msg) ;// reading 1 == msg type
                handle_msg(msg_type, reply); // function that handles msgs according to msg type
            }

            freeReplyObject(reply);

            // wait drone statuses for logs bloccante  (this is for the monitor so it can be blocking)
            reply = read_stream(this->c, log_stream);
            assertReplyType(this->c, reply, REDIS_REPLY_ARRAY);

            //update drones, these info will be used to verify requirements
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
