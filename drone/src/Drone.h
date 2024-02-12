#ifndef DRONE_H
#define DRONE_H
#include "config.h"
#include "../con2redis/src/con2redis.h"
#include <vector>
#include <math.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>

typedef struct {
    int sax; //subarea x 
    int say; //subarea y
    int ny;  // next point of the subarea to start
    int nx;  //
    int dx;  // direction x
    int dy; //usually 0 but changes when doing lawn mowner
    //TODO add dy in everything
} Job;
double get_random_charge_time();
void reset_job(Job * job);
void get_job_msg(int sax,int say, int ny, int nx, int dx, int dy, char *buffer, const char * type ,int id);
Job * get_job_from_reply(redisReply * reply);
enum flying_status {
    FLYING_F,
    LAWN_MOWNER_F,
    WAIT_NEXT_DRONE_F,
    HOMING_F,
    STOP_F
};
enum drone_status{
    STARTUP,
    IDLE,
    FLYING,//arrive at start pos
    WAIT_NEXT_DRONE, // flying to the last pos while other drone arrives
    HOMING, //going home
    CHARGING
};
class Drone {
private:
    // Attributi privati della classe Drone
    int id; // Identificativo unico del drone
    double battery;// Livello della batteria
    double x;
    double y;
    double velx;
    double vely;
    double last_dist; // only when waiting other drone
    drone_status status;
    flying_status f_status;
    double speed; //gets calculated based on how long is time t
    double charge_time;
    // the drone can never be at distance autonomy left 'd' away from its distance from the center 'c'.  if d>=c then the drone needs to go back to charge

public:
    Job * job;
    // Costruttore e metodi pubblici della classe Drone
    Drone(int id);
    void tick(redisContext *c);
    double get_distance_control_center();
    bool is_charged();
    void charge();
    bool is_home();
    bool is_done();
    double get_autonomy();
    bool is_low_battery();
    void set_last_dist();
    void send_replace_drone_msg(redisContext * c);
    void calc_velocity(double destx, double desty);
    void move();
};
class Swarm {
    private:
        int pid;
        redisContext * c; 
        const char * sync_stream = "sync_stream";
        const char * drone_stream = "drone_stream";
        const char * log_stream = "log_stream";

        int block_time = 10000; //timeout redis msgs
    public:
        std::vector<Drone> drones;
        void init();
        void addDrone(Drone drone);
        void init_drones(int n); // initializes the drones
        void await_sync(); //sync time w/ redis
        void tick(); // time tick
        void send_logs(); // logs
        void shutdown();// close connections
};
#endif // DRONE_H
