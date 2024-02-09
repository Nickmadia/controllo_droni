#include "ControlCenter.h"
#include <vector>

// Implementazione dei metodi della classe ControlCenter
ControlCenter::ControlCenter() {/* Costruttore */}
void int_to_string(int x, char * buf ) {
    itoa(x,buf, 10); // convert int to str
}
//TODO add it to the lib
redisReply* ControlCenter::read_stream(redisContext *c, const char *stream_name) {
    // Read from the stream starting from the beginning
    redisReply *reply = (redisReply *)redisCommand(c, "XREAD STREAMS %s 0", stream_name);
    assertReply(c,reply);
    return reply;
}
//TODO add it to the lib
redisReply* ControlCenter::read_1msg(redisContext *c, const char *stream_name) {

    redisReply *reply = (redisReply *)redisCommand(c, "XREAD COUNT 1 STREAMS %s ", stream_name);
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
    char str[8];
    for (int i = 0; i< DRONES_COUNT; i++){
    int_to_string(i, str);
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

    // if needed, dump reply or read it, but shouldnt be necessary atm
    //TODO check wheter the reply contains the sync msg
    freeReplyObject(reply);
    // we could send the time t in the stream or get it calculated from the drones
}
void get_job_msg(const job *my_job, char *buffer ) {
    //TODO remove hardcoded 50
    snprintf(buffer, 50, "sax %d say %d ny %d nx %d dx %d dy %d", 
             my_job->sax, my_job->say, my_job->ny, my_job->nx, my_job->dx, my_job->dy);
}
Job * create_job(int sax, int say, int ny, int nx, int dx) {
    Job* new_job = (job*)malloc(sizeof(job));
    //TODO could be necessary to add dy
    new_job->sax = sax;
    new_job->say = say;
    new_job->dx = dx;
    if (ny < 0|| nx < 0) {
        int subareaside = 20*10;
        if (sax > 3000 && say > 3000) {
            new_job->ny = say;
            new_job->nx = sax;
            new_job->dx = 1;
            new_job->dy = 1;
        } else if (sax < 3000 && say > 3000) {

            new_job->ny = say ;
            new_job->nx = sax + subareaside -1;
            new_job->dx = -1;
            new_job->dy = 1;
        
        } else if (sax < 3000 && say < 3000) {

            new_job->ny = say + subareaside -1; //size of subarea
            new_job->nx = sax + subareaside -1;
            new_job->dx = -1;
            
            new_job->dy = -1;
        } else {

            new_job->ny = say + subareaside -1; //size of subarea
            new_job->nx = sax ;
            new_job->dx = 1;
            new_job->dy = -1;
        }
    }

    new_job->ny = ny;
    new_job->nx = nx;

    return new_job;
}

void ControlCenter::handle_msg(const char * type, redisReply *reply, int id) {
    int msg_type;
    if (strcmp(type,"low_battery") == 0) {
        msg_type = 0;
    } else if (strcmp(type,"charging") == 0) {
        msg_type = 1;
    } else {// recharged
        msg_type = 2;
    }
    switch(msg_type) {
        case 0:
        //low_battery msgtype e.g. type low_battery did 123 cy 0 cx 0 nexty 40 nextx 65 dirx -1
        char say [4];
        char sax [4];
        char ny [4];
        char nx [4];
        char dx [4];
        //create job using next coord. and subarea coord.
        ReadStreamMsgVal(reply,0,0,5,say);
        ReadStreamMsgVal(reply,0,0,7,sax);
        ReadStreamMsgVal(reply,0,0,9,ny);
        ReadStreamMsgVal(reply,0,0,11,nx);
        ReadStreamMsgVal(reply,0,0,13, dx)

        Job *job = create_job(atoi(say),atoi(sax),atoi(ny),atoi(nx),atoi(dx));

        int new_drone_id = get_available_drone_id(); // returns the first free drone

        char n_id[16];
        int_to_string(new_drone_id,n_id);

        freeReplyObject(reply);
        char job_msg[50];
        get_job_msg(job,job_msg);

        reply = (redisReply *)redisCommand(c, "XADD %s * %s", n_id,  job_msg);//job.msg() returns the formated job in a string-type in order to send it via stream
        assertReplyType(c, reply, REDIS_REPLY_STATUS);

        //chagne new drone status
        this->drones[new_drone_id].job = job;
        this->drones[new_drone_id].status = FLYING;
        //change homing drone status
        free(this->drones[id].job );
        this->drones[id].status = HOMING;

        // change lowe_battery_id status to homing
            break;
        case 1:
        //charging msgtype e.g. type charging did 123
        this->drones[id].status = CHARGING;
        // change is status to charging
            break;
        case 2:
        this->drones[id].status = IDLE;
        //recharged msgtype e.g. type recharged
        // change id status to IDLE
            break;
    }
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
            // add drones to the array
                this->addDrone(Drone(drone_id)); // constructor with just the id init drone position at start
            }
            
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

            char job_msg[50];
            //we consider at the start the full swarm to be charged and ready
            char stream_n[16]; //stream number associated with the drone
            redisReply *reply;
            for (int i=0; i<SUB_AREAS_H; i++) {
                for(int j =0; j< SUB_AREAS_W; j++) {
                    //send message to drone this->drones[i*subareasy + subareax] with coord (i,j), sp and sd

                    char *message_id = "*"; // Send to the latest message in the stream
                    // job_msg  e.g. y 80 x 50 spy 90 spx 60 sdx -1 (left) 
                    // also add all of this in a job struct to be associated with the drone obj 

                    int_to_string(i*SUB_AREAS_W + j * 10 *20 -1, stream_n );

                    Job *job = create_job(i,j,-1,-1,-1); // returns a job struct using the subarea coord. use -1 for default

                    get_job_msg(job,job_msg);
                    reply = (redisReply *)redisCommand(c, "XADD %s %s %s", stream_n, message_id, job_msg);//job.msg() returns the formated job in a string-type in order to send it via stream
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
            redisReply *reply ;

            char msg_type[16];

            char stream_n[16]; //stream number associated with the drone
            for( int i =0 ; i< DRONES_COUNT; i++) {
                
                int_to_string(i, stream_n);

                reply = read_1msg(this->c, stream_n );

                assertReplyType(this->c, reply, REDIS_REPLY_ARRAY);
                if (reply-> elements ==0) {
                    freeReplyObject(reply);
                    continue;
                }
                ReadStreamMsgVal(reply, 0, 0, 1, msg_type) ;// reading 1 == msg type

                handle_msg(msg_type, reply ); // function that handles msgs according to msg type
                freeReplyObject(reply);
            }


            
            break;
        default:
            //no status 
            break;
    }
}

void ControlCenter::log() {
    // wait drone statuses for logs bloccante  (this is for the monitor so it can be blocking)

    reply = read_stream(this->c, log_stream);
    assertReplyType(this->c, reply, REDIS_REPLY_ARRAY);
    //drone log e.g. did 123 y 75 x 100 battery 89 

    //after updating the drones array update the grid based on drone positions in the msg
    
    // get monitors bool val on the data and log on csv file

    // use a csv file no need to use postgresql 
}

// clean up resources by deleting streams and free the context
void ControlCenter::shutdown() {
    redisReply *reply;
    char stream_n[16]; //stream number associated with the drone
    for(int i=0; i<DRONES_COUNT; i++) {
        int_to_string(i, stream_n);
        reply = RedisCommand(c, "DEL %s", stream_n);
        assertReply(c, reply);
        freeReplyObject(reply);
    }

    reply = redisCommand(c, "DEL %s", drone_stream);
    assertReply(c,reply);
    freeReplyObject(reply);
    
    reply = redisCommand(c, "DEL %s", sync_stream);
    assertReply(c,reply);
    freeReplyObject(reply);

    redisFree(this-> c);


}

