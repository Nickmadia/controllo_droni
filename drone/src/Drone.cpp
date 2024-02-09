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
    velx = max_speed * unit_dx;
    vely = max_speed * unit_dy;
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
    switch(this->f_status) {
        case FLYING: 
        case HOMING:
            this->calc_velocity(job->nx, job->ny); 

            if (x == job->nx && y == job->ny ) {
                this->f_status = LAWN_MOWNER;
            }
            break;
        case LAWN_MOWNER:
        case WAIT_NEXT_DRONE:
            if (job->dx == -1) {
                double m_speed = (speed >= x - job->sax )? (x-job->sax) : speed;
                this->x += m_speed * job.dx;
            } else {
                double m_speed = (speed >= job->sax +200  - x)? (job->sax +200 -x) : speed;
                this->x += m_speed * job.dx;
            }
             
            if(x == job->sax || x == job->sax + 200) {
                job->ny += job->dy * 20;
                job->dx = -job->dx;
                if(job->ny <= job->say || job->ny >= job->say +200) {
                    this->f_status = FLYING;
                    reset_job(this->job);
                }
                break;

            }

            //check if hits bound
            break;
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
            this->move(); // if flying do smthing else if waitnext drone do another thing

            // if arrived calc next using lawn mowner algo

            //if current_autonomy <= d_control center x2 + 2margin (max dist the drone can get while waiting other drone while keep moving)
            if(this->is_low_battery()) {
            // calculate last poss for this drone
            // calc next poss for the next drone (should be cur_pos + d_control_cent ofc applied on the grid)
            this->status = WAIT_NEXT_DRONE;
            }



            break;
        case WAIT_NEXT_DRONE: //add to f status
            this->move(); 
            // vai alla pos lastx lasty
            if(this->is_done()) {
                this->status = HOMING;
                //send msg
            }
            //if last job pos == pos
                //go to homing
            break;
        case HOMING:
            // move home
            this-> move();

            if(this-> is_home()) {
                this->status = CHARGING;

            }
            // if home go charging
            // send msg
            break;
        case CHARGING:
            //charge
            this->charge();
            if (this->is_charged()) {
                this->status = IDLE;
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