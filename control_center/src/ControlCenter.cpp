#include "ControlCenter.h"
#include <vector>
void ControlCenter::print_status() {
    printf("status : %d\n",status);
}
void print_parameters() {
    printf("Horizon : %d\nDrones count: %d\n", HORIZON,DRONES_COUNT);
}
// Implementazione dei metodi della classe ControlCenter
ControlCenter::ControlCenter() {/* Costruttore */}
const char * int_to_string(int x) {
    return std::to_string(x).c_str();
}
//TODO add it to the lib
redisReply* read_stream(redisContext *c, const char *stream_name) {
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
    status = STARTUP;
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
    reply = (redisReply *)redisCommand(c, "DEL %s", sync_stream);
    assertReplyType(c, reply, REDIS_REPLY_INTEGER);
        initStreams(c, sync_stream);
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(c, "DEL %s", "sync_stream2");
    assertReplyType(c, reply, REDIS_REPLY_INTEGER);
        initStreams(c, "sync_stream2");
    freeReplyObject(reply);

    //set up stream for receiving from drone
    reply = (redisReply *)redisCommand(c, "DEL %s", drone_stream);
    assertReplyType(c, reply, REDIS_REPLY_INTEGER);
        initStreams(c , drone_stream);
    freeReplyObject(reply);
    

    //set up streams to send to a single drone create one stream for each drone
    for (int i = 0; i< DRONES_COUNT; i++){
    const char *str = int_to_string(i);
    printf("key : %s\n", str);
    reply = (redisReply *)redisCommand(c, "DEL %d", i);
    assertReplyType(c, reply, REDIS_REPLY_INTEGER);
    freeReplyObject(reply);
    //initStreams(c , str);
    reply = RedisCommand(c, "XGROUP CREATE %d diameter $ MKSTREAM", i);
    dumpReply(reply,0);
    assertReply(c, reply);

    freeReplyObject(reply);
    }


}
int ControlCenter::get_available_drone_id(){
    auto is_available = [](Drone  drone){return (drone.status == IDLE_D);};
    auto drone = std::find_if(drones.begin(),drones.end(), is_available);
    return drone->id;
}
void ControlCenter::await_sync() {
    redisReply *reply;

    // Send a synchronization message to the Redis stream
    const char *message_id = "*"; // Send to the latest message in the stream
    const char *message = "Sync";
    reply = (redisReply *)redisCommand(c, "XADD %s %s type %s", "sync_stream2", message_id, message);
    if(DEBUG) {
        dumpReply(reply,0);
    }
    assertReplyType(c, reply,REDIS_REPLY_STRING );
    //if needed log reply later
    freeReplyObject(reply);

    // Block to receive synchronization messages from the Redis stream
    //read sync_stream1
    reply = read_1msg_blocking(c, "diameter", "cc", block_time, sync_stream);
    assertReply(c, reply);
    dumpReply(reply,0);
    // if needed, dump reply or read it, but shouldnt be necessary atm
    //TODO check wheter the reply contains the sync msg
    freeReplyObject(reply);
    // we could send the time t in the stream or get it calculated from the drones
}
void get_job_msg(Job *my_job, char *buffer ) {
    //TODO remove hardcoded 50
    snprintf(buffer, 50, "sax %d say %d ny %d nx %d dx %d dy %d", 
             my_job->sax, my_job->say, my_job->ny, my_job->nx, my_job->dx, my_job->dy);
}
Job * create_job(int say, int sax, int ny, int nx, int dx, int dy) {
    Job* new_job = (Job*)malloc(sizeof(Job));
    //TODO could be necessary to add dy
    new_job->sax = sax;
    new_job->say = say;
    if (ny < 0|| nx < 0) {
        int subareaside = 20*10;
        if (sax > 3000 && say > 3000) {
            new_job->ny = say + 10;
            new_job->nx = sax + 10;
            new_job->dx = 1;
            new_job->dy = 1;
        } else if (sax < 3000 && say > 3000) {

            new_job->ny = say +10;
            new_job->nx = sax + subareaside -10 ;
            new_job->dx = -1;
            new_job->dy = 1;
        
        } else if (sax < 3000 && say < 3000) {

            new_job->ny = say + subareaside -10; //size of subarea
            new_job->nx = sax + subareaside -  10;
            new_job->dx = -1;
            
            new_job->dy = -1;
        } else {

            new_job->ny = say + subareaside - 10 ; //size of subarea
            new_job->nx = sax + 10 ;
            new_job->dx = 1;
            new_job->dy = -1;
        }
        return new_job;
    }

    new_job->dx = dx;
    new_job->dy = dy;
    new_job->ny = ny;
    new_job->nx = nx;

    return new_job;
}

void ControlCenter::handle_msg(const char * type, redisReply *reply ) {
    int msg_type;
    if (strcmp(type,"low_battery") == 0) {
        msg_type = 0;
    } else if (strcmp(type,"charging") == 0) {
        msg_type = 1;
    } else {// recharged
        msg_type = 2;
    }
    switch(msg_type) {
        case 0: {
        //low_battery msgtype e.g. type low_battery did 123 cy 0 cx 0 nexty 40 nextx 65 dirx -1
        char say [4];
        char sax [4];
        char ny [4];
        char nx [4];
        char dx [4];
        char dy [4];
        char did [4];
        //create job using next coord. and subarea coord.
        ReadStreamMsgVal(reply,0,0,3,did);
        ReadStreamMsgVal(reply,0,0,5,say);
        ReadStreamMsgVal(reply,0,0,7,sax);
        ReadStreamMsgVal(reply,0,0,9,ny);
        ReadStreamMsgVal(reply,0,0,11,nx);
        ReadStreamMsgVal(reply,0,0,13, dx);
        ReadStreamMsgVal(reply,0,0,15, dy);
        Job *job = create_job(atoi(say),atoi(sax),atoi(ny),atoi(nx),atoi(dx), atoi(dy));

        int new_drone_id = get_available_drone_id(); // returns the first free drone


        printf(">>>>>>>>>sending %d drone\n",new_drone_id);
        freeReplyObject(reply);

        //TODOTODOTODO IMPORTANT
        reply = (redisReply *)redisCommand(c, "XADD %d * type %s did %d say %d sax %d ny %d nx %d dx %d dy %d"
                                            , new_drone_id, "task", new_drone_id, job->say,
                                             job->sax, job->ny, job->nx, job->dx, job->dy );//job.msg() returns the formated job in a string-type in order to send it via stream
        assertReplyType(c, reply, REDIS_REPLY_STRING);
        dumpReply(reply,0);

        //chagne new drone status
        this->drones[new_drone_id].job = job;
        this->drones[new_drone_id].status = FLYING_D;
        //change homing drone status
        free(this->drones[atoi(did)].job );
        this->drones[atoi(did)].status = HOMING_D;

        // change lowe_battery_id status to homing
            break;
        }
        //charging msgtype e.g. type charging did 123
        char did [4];
        ReadStreamMsgVal(reply,0,0,3,did);
        this->drones[atoi(did)].status = CHARGING_D;

        // change is status to charging
        }
            break;
        case 2: {
        char did [4];
        ReadStreamMsgVal(reply,0,0,3,did);
        this->drones[atoi(did)].status = IDLE_D;
        //recharged msgtype e.g. type recharged did 123
        // change id status to IDLE
            break;
        }
    }
}

void ControlCenter::tick() {
    switch (status) {
        case STARTUP: {
            //startup
            printf("startup\n");
            redisReply *reply ;
            //redisReply *msgs = reply->element[0];// reading from only one stream so [0]

            // get elements as droneid, 
            char value[8];
            for( int i =0 ; i< DRONES_COUNT; i++) {
                //should check type remember later + format
                // first field = type, 2nd drone_id
                /* define msg types later as integers
                    wake = 0 ,

                    */
                // field value example should be 0 , 0
                if(DEBUG) {
                    printf("drone n: %d\n",i);
                }
                reply = read_1msg_blocking(c,"diameter", "cc",10000, drone_stream);
                assertReply(this->c, reply );
                if(reply->type != REDIS_REPLY_NIL && reply->elements>0){
                    dumpReply(reply,0);
                    ReadStreamMsgVal(reply, 0, 0, 3, value) ;// reading 3rd element of the msg
                    int drone_id = std::stoi(value);;
                    // add drones to the array
                    this->addDrone(Drone(drone_id)); // constructor with just the id init drone position at start

                }
                freeReplyObject(reply);
            }
            
            if (this->drones.size() == DRONES_COUNT) {//add DRONES COUNT TO CC attr, get it from command line
                this->status = READY;
            }
            break;
        }
        case READY: {
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
            redisReply *reply;
            for (int i=0; i<2; i++) {
                for(int j =0; j< 2; j++) {
                    //send message to drone this->drones[i*subareasy + subareax] with coord (i,j), sp and sd

                    // job_msg  e.g. y 80 x 50 spy 90 spx 60 sdx -1 (left) 
                    // also add all of this in a job struct to be associated with the drone obj 

                    int did = i *SUB_AREAS_W + j;
                    const char * stream_n = int_to_string( did  );//stream number associated with the drone

                    Job *job = create_job(i * 200,j * 200,-1,-1,-1,-1); // returns a job struct using the subarea coord. use -1 for default

                    reply = (redisReply *)redisCommand(c, "XADD %s * type %s did %d say %d sax %d ny %d nx %d dx %d dy %d"
                                            , stream_n, "task", did, job->say,
                                             job->sax, job->ny, job->nx, job->dx, job->dy );//job.msg() returns the formated job in a string-type in order to send it via stream
                    dumpReply(reply,0);
                    assertReplyType(c, reply, REDIS_REPLY_STRING);

                    freeReplyObject(reply);
                    // change drone status to running
                    this->drones[i*SUB_AREAS_H + j].job = job;
                    this->drones[i*SUB_AREAS_H + j].status = FLYING_D;

                }
            }
            // assegna ogni area ad un drone

            //go to running
            if(DEBUG) printf("changing to running in 3sec\n");
            this->status = RUNNING;
            break;
        }
        case RUNNING: {
            //running
            //read messages

            // once the cc is up and running, it only recevices msgs and responds accordingly to each one
            
            //read all msgs in the stream at time t
            redisReply *reply ;

            char msg_type[30];

            for( int i =0 ; i< DRONES_COUNT; i++) {
                
                //const char * stream_n = int_to_string(i);
                printf("drone  : %d\n", i);
               // reply = read_1msg(this->c, "diameter", "cc", *stream_n );
                reply = (redisReply *)redisCommand(c, "XREADGROUP GROUP %s %s COUNT 1 NOACK STREAMS %s >", 
                                    "diameter", "cc",  drone_stream);
                assertReply(c,reply);

                dumpReply(reply,0);
                if (reply->type == REDIS_REPLY_NIL || reply-> elements ==0 ) {
                    freeReplyObject(reply);
                    continue;
                }
                ReadStreamMsgVal(reply, 0, 0, 1, msg_type) ;// reading 1 == msg type

                handle_msg(msg_type, reply ); // function that handles msgs according to msg type
                freeReplyObject(reply);
            }


            
            break;
        }
        default:
            //no status 
            break;
    }
}
bool ControlCenter::check_area(int curr_time){
    for (int i = 0 ; i < HEIGHT; i++) {
            for (int j =0 ; j < WIDTH; j++) {
                if(!is_verified(grid[i][j], curr_time)){
                    return false;
                }
            }
    }
    return true;
}

bool is_verified(int last_v, int current_time){
    return (double)(current_time -last_v)/ T <= 300.0 / T;
}
void ControlCenter::log(int time) {
    // wait drone statuses for logs bloccante  (this is for the monitor so it can be blocking)
    redisReply * reply;

    //drone log e.g. did 123 battery 89 lvy 30 lvx 90 tv 340 status 0
    for(int i= 0; i<DRONES_COUNT;i++){
        reply = read_1msg_blocking(c,"diameter","cc", 10000, log_stream) ;
        char drone_id [4];
        char bat [4];
        char lvy [4];
        char lvx [4];
        char tv [4];
        //create job using next coord. and subarea coord.
        //TODO if drone idle continue
        ReadStreamMsgVal(reply,0,0,1,drone_id);
        ReadStreamMsgVal(reply,0,0,3,bat);
        ReadStreamMsgVal(reply,0,0,5,lvy);
        ReadStreamMsgVal(reply,0,0,7,lvx);
        ReadStreamMsgVal(reply,0,0,9,tv);

        int did = std::stoi(drone_id);

        drones[did].battery = std::stod(bat);

        // MONITOR BATTERIA
        assert(std::stod(bat) > MIN_BATTERY);
        drone[did].last_verified_x = std::stoi(lvx);
        drone[did].last_verified_y = std::stoi(lvy);
        int ltv = std::stoi(tv);

        if (grid[lvy][lvx] != ltv) {
            grid[lvy][lvx] = ltv;
        }



        // optimize

    }

    bool area_verified1 = area_verified; //verfica allo stato precedente
    area_verified = check_area();

    //MONITOR AREA
    assert( area_verified1 ? area_verified: 1); // se l' area diventa verificata non deve mai smettere

    // MONITOR TIME 
    assert( MAX_VERIFY_TIME >= ) 
    

    //after updating the drones array update the grid based on drone positions in the msg
    
    // get monitors bool val on the data and log on csv file

    // use a csv file no need to use postgresql 
}

// clean up resources by deleting streams and free the context
void ControlCenter::shutdown() {
    redisReply *reply;
    for(int i=0; i<DRONES_COUNT; i++) {
        const char * stream_n = int_to_string(i);
        reply = (redisReply *)RedisCommand(c, "DEL %s", stream_n);
        assertReply(c, reply);
        freeReplyObject(reply);
        free(drones[i].job);
    }

    reply = (redisReply *)redisCommand(c, "DEL %s", drone_stream);
    assertReply(c,reply);
    freeReplyObject(reply);
    
    reply = (redisReply *)redisCommand(c, "DEL %s", sync_stream);
    assertReply(c,reply);
    freeReplyObject(reply);

    redisFree(this-> c);


}

