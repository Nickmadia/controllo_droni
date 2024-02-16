#ifndef CONTROL_CENTER_H
#define CONTROL_CENTER_H
#include <vector>
#include <time.h>
#include "config.h"
#include "../con2redis/src/con2redis.h"
#include <math.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <algorithm>

void print_parameters();
const char * int_to_string(int x);
typedef struct {
    int sax; //subarea x 
    int say; //subarea y
    int ny;  // next point of the subarea to start
    int nx;  //
    int dx;  // direction x
    int dy;
} Job;
void get_job_msg(const Job *my_job, char *buffer );
Job * create_job(int sax, int say, int ny, int nx, int dx, int dy);

enum drone_status{
    IDLE_D,
    FLYING_D,
    HOMING_D,
    CHARGING_D
};


class Drone {
private:
    int last_verified_x; // last verified pos 
    int last_verified_y; // last verified post 
    
    // pos x will be t.c. (x -10) congruo 0 mod 20
    double battery; // Livello della batteria del drone

public:
    // Costruttore
    Drone( int did) {
        this->id = did;
        x = 3000;
        y = 3000;
        battery = 100;
        status = IDLE_D;
    }
    int id;
    drone_status status; // Stato del drone
    Job * job;
};
enum Status {
    STARTUP,
    READY,
    RUNNING,
    STATUS_COUNT
};
class ControlCenter {
private:
    int pid;
    std::vector<Drone> drones;// vector of drones ids, make another drone class
    int grid[WIDTH][HEIGHT];// griglia 2d of time stamps in unix epoch, will be used to calculate verified time
    Status status; 
    bool area_verified;
    redisContext *c;// redis conn
    const char * sync_stream = "sync_stream1";
    const char * drone_stream = "drone_stream";
    const char * log_stream = "log_stream";
    int block_time = 10000000; //timeout redis msgs

public:
    ControlCenter();
    void print_status();
    void addDrone(Drone drone);
    void init();
    void await_sync();
    void tick();
    void handle_msg(const char * type, redisReply * reply);
    void log();
    void shutdown();

    void create_subarea(); // divides the area current(600x600) in sub_areas to be assigned to the individual drones
    void divide_tasks(); // assign the subareas to the available drones
    int get_available_drone_id();

    // sub areas must be of a max of 250 points - or if sqrt(250) = l, lxl -1 // 
    bool check_area(); // checks whether a point has been verified in the last 5 min, if not logs functional requisite has been violeted - checks every point on the grid 
    bool is_verified(); // checks single point




    // ...
};

//need a dummy drone class

#endif // CONTROL_CENTER_H
