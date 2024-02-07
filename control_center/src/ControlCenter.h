#ifndef CONTROL_CENTER_H
#define CONTROL_CENTER_H
#include <vector>
#include <time.h>


enum drone_status{
    STARTUP,
    IDLE,
    FLYING,
    HOMING,
    CHARGING
};

class Drone {
private:
    float x; // Coordinata x del drone
    float y; // Coordinata y del drone
    float battery; // Livello della batteria del drone
    drone_status status; // Stato del drone

public:
    // Costruttore
    Drone(int initialX, int initialY, float initialBattery, Status initialStatus) {
        x = initialX;
        y = initialY;
        battery = initialBattery;
        status = initialStatus;
    }
};
enum Status {
    STARTUP,
    READY,
    RUNNING,
    STATUS_COUNT
};
class ControlCenter {
private:
    std::vector<Drone> drones;// vector of drones ids, make another drone class
    time_t grid[WIDTH][HEIGHT];// griglia 2d of time stamps in unix epoch, will be used to calculate verified time
    Status status; 
    redisContext *c;// redis conn
    const char * sync_stream = "sync_stream";
    const char * drone_stream = "drone_stream";
    int block_time = 10000000; //timeout redis msgs

public:
    ControlCenter();
    void init();
    void await_sync();
    void tick();

    void create_subarea(); // divides the area current(600x600) in sub_areas to be assigned to the individual drones
    void divide_tasks(); // assign the subareas to the available drones
    int get_available_drone();
    bool send_instruction(int id); //sends the drone an istruction to start a task

    // sub areas must be of a max of 250 points - or if sqrt(250) = l, lxl -1 // 
    void check_area(); // checks whether a point has been verified in the last 5 min, if not logs functional requisite has been violeted - checks every point on the grid 
    bool is_verified(); // checks single point
    void run(); //main function, it has the loop on every drone at every istant of time t




    // ...
};

//need a dummy drone class

#endif // CONTROL_CENTER_H
