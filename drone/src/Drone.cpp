#include "Drone.h"
const char *int_to_string(int x) {
    return std::to_string(x).c_str();
}
redisReply* read_stream(redisContext *c, const char *stream_name) {
    // Read from the stream starting from the beginning
    redisReply *reply = (redisReply *)redisCommand(c, "XREAD STREAMS %s 0", stream_name);
    assertReply(c,reply);
    return reply;
}

// Implementazione dei metodi della classe Drone
Drone::Drone(int id) {
    id = id;
    battery = 100, 
    x= 3000;
    y= 3000;
    speed = 0;
    status = STARTUP;
 }
void Swarm::addDrone(Drone drone) {
    // Aggiunge un drone al centro di controllo
    drones.push_back(drone);
}
void Swarm::init_drones(int n) {
    for (int i =0; i<n ; i++){
        addDrone(Drone(i));
    }
}
void Swarm::await_sync() {
    redisReply *reply;


    // Block to receive synchronization messages from the Redis stream
    //reply = (redisReply *)RedisCommand(c, "XREAD COUNT 1 BLOCK %d STREAMS %s >", block_time, sync_stream);

    reply = read_1msg(c, "diameter", "sync",block_time, sync_stream);
    dumpReply(reply,0);

    // if needed, dump reply or read it, but shouldnt be necessary atm
    //TODO check wheter the reply contains the sync msg
    //freeReplyObject(reply);
    // we could send the time t in the stream or get it calculated from the drones

    // Send a synchronization message to the Redis stream
    //char *message_id = "*"; // Send to the latest message in the stream
    char *message = "sync";
    reply = (redisReply *) RedisCommand(c, "XADD %s * type %s", sync_stream, message);
    //reply = (redisReply *)redisCommand(c, "XADD %s %s type %s", sync_stream, message_id, message);
    if(DEBUG) {
        dumpReply(reply,0);
        assertReplyType(c, reply, REDIS_REPLY_STRING);
    }
    //if needed log reply later
    freeReplyObject(reply);
}
void Swarm::init() {
    //set up redis context
    this->pid = getpid();
    srand( time(NULL));
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
    if(DEBUG) {
        printf("initialized redis con\n");
        redisReply *reply = (redisReply*)redisCommand(c, "PING");
        if (reply == NULL) {
            printf("Error: Failed to execute PING command\n");
            redisFree(c);
            exit(1);
        }

        // Check the reply
        if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "PONG") == 0) {
            printf("Redis is alive!\n");
        } else {
            printf("Error: Redis did not respond properly\n");
        }

        freeReplyObject(reply);
    }
}
Job * get_job_from_reply(redisReply *reply){
    //low_battery msgtype e.g. type low_battery did 123 cy 0 cx 0 nexty 40 nextx 65 dirx -1 diry -1
        char say [4];
        char sax [4];
        char ny [4];
        char nx [4];
        char dx [4];
        char dy [4];
        //create job using next coord. and subarea coord.
        ReadStreamMsgVal(reply,0,0,5,say);
        ReadStreamMsgVal(reply,0,0,7,sax);
        ReadStreamMsgVal(reply,0,0,9,ny);
        ReadStreamMsgVal(reply,0,0,11,nx);
        ReadStreamMsgVal(reply,0,0,13, dx);
        ReadStreamMsgVal(reply,0,0,15, dy);

        Job *job = (Job *)malloc(sizeof(Job));

        // Assign values to the members
        job->sax = atoi(sax);
        job->say = atoi(say);
        job->ny = atoi(ny);
        job->nx = atoi(nx);
        job->dx = atoi(dx);
        job->dy = atoi(dy); 
        return job;
}
double get_random_charge_time() {
    // Generate a random number between min and max
    double min = 2 * 60 *60;
    double max = 3 *60 * 60 ;
    double randomNumber = (double)rand() / RAND_MAX * (max - min) + min;
    
    return randomNumber;
}
double Drone::get_distance_control_center(){
    return sqrt(pow(x-3000,2) + pow(y-3000,2));
}
bool Drone::is_charged(){
    return charge_time <= 0;
}
void Drone::charge(){
    charge_time -= T; 
}
bool Drone::is_home() {
    return x == 3000 && y == 3000;
}
bool Drone::is_done() {
    return last_dist <= 0;
}
double Drone::get_autonomy(){
    return battery /100 * MAX_DISTANCE;
}
bool Drone::is_low_battery(){
    return get_autonomy() <= get_distance_control_center() *2 + MARGIN;//TODO implement get_dist
}
void Drone::set_last_dist() {
    last_dist = get_distance_control_center();
}

void Drone::send_replace_drone_msg(redisContext * c) {
    // msg e.g. type low_battery did 123 cy 20 cx 20 ny 40 nx 40 dx -1 dy -1
    double d_last;
    if(job->dy) { //we are on the down side of the grid
        d_last = (  job->say +200 - y) *200 ;
    } else {
        d_last = (y - job->say);
    }
    if (job->dx){ //bottom right
        d_last += (x - job->sax);
    } else {
        d_last += (job->sax -x);
    }
    int temp = last_dist -d_last;

    int move_y = (temp/11) %10;
    int nexty;
    if (job->dy) {
        nexty =(int) job->say + move_y *20;
    } else {
        nexty =(int) job->say +200 - move_y *20;
    }

    int move_x = (temp % 10);

    int ndx = ((nexty /20)%2) ? 1:-1;
    ndx = job->sax<=3000? ndx : -ndx;

    int nextx;
    if(!job->dx) {
        nextx = job->sax + move_x *20;
    } else {
        nextx = job->sax + 200 - move_x *20;
    }
    redisReply * reply = (redisReply *)redisCommand(c, "XADD %d * %s type %s did %d say %d sax %d ny %d nx %d dy %d dx %d"
                                    , id, "low_battery", id, job->say, job->sax, job->ny
                                    , job->nx, job->dy, job->dx  );//
    assertReplyType(c, reply, REDIS_REPLY_STRING);
    freeReplyObject(reply);

}
void Drone::calc_velocity(double destx, double desty) {

    // Normalize the direction vector
    double diffX = destx - x;
    double diffY = desty - y;
    

    // Normalize the differences to obtain unit vectors
    double length = sqrt(diffX * diffX + diffY * diffY); // Length of the vector

    double max_speed = (speed >= length)? length : speed;

    double unitX = diffX / length;
    double unitY = diffY / length;

    // Calculate velocity components
    velx = max_speed * unitX;
    vely = max_speed * unitY;
}
void reset_job(Job * job) {
    int subareaside = 20*10;
        if (job->sax > 3000 && job->say > 3000) {
            job->ny = job->say;
            job->nx = job->sax;
            job->dx = 1;
            job->dy = 1;
        } else if (job->sax < 3000 && job->say > 3000) {

            job->ny = job->say ;
            job->nx = job->sax + subareaside -1;
            job->dx = -1;
            job->dy = 1;
        
        } else if (job->sax < 3000 && job->say < 3000) {

            job->ny = job->say + subareaside -1; //size of subarea
            job->nx = job->sax + subareaside -1;
            job->dx = -1;
            
            job->dy = -1;
        } else {

            job->ny = job->say + subareaside -1; //size of subarea
            job->nx = job->sax ;
            job->dx = 1;
            job->dy = -1;
        }
}
void Drone::move() {
    this-> battery -= 100/(1800/T); //TODO 
    flying_status *next_f_status = NULL;
    switch(this->f_status) {
        case FLYING_F: 
            this->calc_velocity(job->nx, job->ny); 
            x += velx;
            y += vely;
            if (x == job->nx && y == job->ny ) {
                *next_f_status = LAWN_MOWNER_F;
            }
            break;
        case HOMING_F:
            this->calc_velocity(job->nx, job->ny); 
            x += velx;
            y += vely;
            break;
        case WAIT_NEXT_DRONE_F:
        case LAWN_MOWNER_F:
            double m_speed;
            if (job->dx == -1) {
                m_speed = (speed >= x - job->sax )? (x-job->sax) : speed;
                this->x += m_speed * job->dx;
            } else {
                m_speed = (speed >= job->sax +200  - x)? (job->sax +200 -x) : speed;
                this->x += m_speed * job->dx;
            }
            if(f_status == WAIT_NEXT_DRONE_F) {
                last_dist -= m_speed;
            }
            if(x == job->sax || x == job->sax + 200) {
                job->ny += job->dy * 20;
                job->dx = -job->dx;
                if(job->ny <= job->say || job->ny >= job->say +200) {
                    *next_f_status = FLYING_F;
                    reset_job(this->job);
                }
            }
    }
    if(next_f_status != NULL){
        f_status = *next_f_status;
    }
}
void Drone::tick(redisContext *c) {
    redisReply *reply;
    switch(this->status) {
        case STARTUP: {
            printf("startup\n");
            //send awake msg
            //const char *message_id = "*"; // Send to the latest message in the stream
            const char * stream_name = "drone_stream";

            reply = (redisReply*) RedisCommand(c, "XADD %s * type awk did %d", stream_name, id);
            dumpReply(reply,0);
            assertReplyType(c, reply, REDIS_REPLY_STRING);

            freeReplyObject(reply);

            status = IDLE;
            break;
        }
        case IDLE: {
            
            const char *buf = int_to_string(this->id ) ;//TODO lib remember to add
            reply = read_1msg(c, "diameter", buf ,buf); 
            assertReply(c, reply );
            dumpReply(reply,0);
            if (reply-> elements ==0) {
                if(DEBUG){
                    printf("idle: zero elements\n");
                }
                freeReplyObject(reply);
                break;
            }

            this->job = get_job_from_reply(reply);

            status = FLYING;
            freeReplyObject(reply);
            //wait job
            //if job go FLYING
            break;
        }
        case FLYING: {
            // vai alla pos nx ny with velx vely

            // if arrived calc next using lawn mowner algo

            //if current_autonomy <= d_control center x2 + 2margin (max dist the drone can get while waiting other drone while keep moving)
            if(this->is_low_battery()) {
            // calculate last poss for this drone
            // calc next poss for the next drone (should be cur_pos + d_control_cent ofc applied on the grid)
            this->set_last_dist();
            this->send_replace_drone_msg(c);
            status= WAIT_NEXT_DRONE;
            this->f_status = WAIT_NEXT_DRONE_F;
            break;
            }

            this->move(); // if flying do smthing else if waitnext drone do another thing


            break;
        }
        case WAIT_NEXT_DRONE: {//add to f status
            if(this->is_done()) {
                this->job->nx = 3000;
                this->job->ny = 3000;
                status = HOMING;
                this->f_status = HOMING_F;
                //send msg
            }
            this->move(); 
            // vai alla pos lastx lasty
            break;
        }
        case HOMING:{
            // move home
            if(this-> is_home()) {
                status = CHARGING;
                this-> charge_time = get_random_charge_time();
                free(this->job);
                break;
            }
            this-> move();

            // if home go charging
            // send msg
            break;
        }
        case CHARGING: {
            //charge
            this->charge();
            if (this->is_charged()) {
                status = IDLE;
                this->battery = 100;
                //send mesg
            }
            //if charged go idle
            // send msg
            printf("Drone status: CHARGING\n");
            break;
        }
        default:
            printf("Unknown drone status\n");
            break;
    }
}
void Swarm::tick() { //performs tick for all drones
    for(int i=0; i < DRONES_COUNT; i++ ) {
        drones[i].tick(c);
    }
}

void Swarm::shutdown() {
    for(int i =0; i< DRONES_COUNT; i++) {
        free(drones[i].job);
    }
    redisFree(c);
}