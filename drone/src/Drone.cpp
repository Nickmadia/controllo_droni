#include "Drone.h"

// Implementazione dei metodi della classe Drone
Drone::Drone(int id) :  {
    drone_id = id;
    battery_level = 100, 
    is_charging= false,
    x= 0;
    y= 0;
    speed = 0;
    control_radius = 10;
 }
void swarm::addDrone(Drone drone) {
    // Aggiunge un drone al centro di controllo
    drones.push_back(drone);
}
void Swarm::init_drones(int n) {
    for (int i =0; i<n ; i++){
        addDrone(Drone(i));
    }
}
void Swarm::init() {
    //set up redis context
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
}
Job * get_job_from_reply(redisReply *reply){
    //low_battery msgtype e.g. type low_battery did 123 cy 0 cx 0 nexty 40 nextx 65 dirx -1
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
void Drone::is_charged(){
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
bool Drone::is_low_battery(){
    return get_autonomy() <= get_distance_control_center() *2 + margin;
}
void Drone::calc_velocity(int destx, int desty) {

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
        } else if (sax < 3000 && say > 3000) {

            job->ny = job->say ;
            job->nx = job->sax + subareaside -1;
            job->dx = -1;
            job->dy = 1;
        
        } else if (sax < 3000 && say < 3000) {

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
    this-> battery_level -= 100/(1800/0.1); //TODO 
    switch(this->f_status) {
        case FLYING: 
            this->calc_velocity(job->nx, job->ny); 
            x += velx;
            y += vely;
            if (x == job->nx && y == job->ny ) {
                this->f_status = LAWN_MOWNER;
            }
            break;
        case HOMING:
            this->calc_velocity(job->nx, job->ny); 
            x += velx;
            y += vely;
            break;
        case WAIT_NEXT_DRONE:
        case LAWN_MOWNER:
            if (job->dx == -1) {
                double m_speed = (speed >= x - job->sax )? (x-job->sax) : speed;
                this->x += m_speed * job.dx;
            } else {
                double m_speed = (speed >= job->sax +200  - x)? (job->sax +200 -x) : speed;
                this->x += m_speed * job.dx;
            }
            if(f_status == WAIT_NEXT_DRONE) {
                last_dist -= m_speed;
            }
            if(x == job->sax || x == job->sax + 200) {
                job->ny += job->dy * 20;
                job->dx = -job->dx;
                if(job->ny <= job->say || job->ny >= job->say +200) {
                    this->f_status = FLYING;
                    reset_job(this->job);
                }
            }
    }
}
void Drone::tick(redisContext *c) {
    switch(this->status) {
        case STARTUP:
            //send awake msg
            redisReply *reply;
            const char *message_id = "*"; // Send to the latest message in the stream
            const char *message = "awake";
            
            reply = (redisReply *)redisCommand(c, "XADD %s %s type %s did %d", "drone_stream", message_id, message, this->id);
            assertReplyType(c, reply, REDIS_REPLY_STATUS);
            freeReplyObject(reply);

            this->status = IDLE;
            break;
        case IDLE:
            redisReply *reply;
            
            char buf[8];
            int_to_string(this->id, buf) ;//TODO lib remember to add
            reply = read_1msg(c, buf); //TODO func from lib, remember to add it
            assertReplyType(this->c, reply, REDIS_REPLY_ARRAY);
            if (reply-> elements ==0) {
                freeReplyObject(reply);
                continue;
            }

            this->job = get_job_from_reply(reply);

            this->status = FLYING;
            freeReplyObject(reply);
            //wait job
            //if job go FLYING
            break;
        case FLYING:
            // vai alla pos nx ny with velx vely

            // if arrived calc next using lawn mowner algo

            //if current_autonomy <= d_control center x2 + 2margin (max dist the drone can get while waiting other drone while keep moving)
            if(this->is_low_battery()) {
            // calculate last poss for this drone
            // calc next poss for the next drone (should be cur_pos + d_control_cent ofc applied on the grid)
            this->status = WAIT_NEXT_DRONE;
            this->f_status = WAIT_NEXT_DRONE;
            break;
            }

            this->move(); // if flying do smthing else if waitnext drone do another thing


            break;
        case WAIT_NEXT_DRONE: //add to f status
            if(this->is_done()) {
                this->job.nx = 3000;
                this->job.ny = 3000;
                this->status = HOMING;
                this->f_status = HOMING;
                //send msg
            }
            this->move(); 
            // vai alla pos lastx lasty
            break;
        case HOMING:
            // move home
            if(this-> is_home()) {
                this->status = CHARGING;
                this-> charge_time == get_random_charge_time();
                break;
            }
            this-> move();

            // if home go charging
            // send msg
            break;
        case CHARGING:
            //charge
            this->charge();
            if (this->is_charged()) {
                this->status = IDLE;
                this->battery_level == 100;
                //send mesg
            }
            //if charged go idle
            // send msg
            printf("Drone status: CHARGING\n");
            break;
        default:
            printf("Unknown drone status\n");
            break;
    }
}
void Swarm::tick() { //performs tick for all drones
    for(int i=0; i < DRONES_COUNT; i++ ) {
        drones[i].tick();
    }
}