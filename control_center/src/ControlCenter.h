#ifndef CONTROL_CENTER_H
#define CONTROL_CENTER_H
#include <vector>
#include <time.h>
#define WIDTH 600 //since we want points 10m apart
#define HEIGHT 600


class ControlCenter {
private:
    std::vector<Drone> drones;// vector of drones ids, make another drone class
    time_t * grid[WIDTH][WIDTH];// griglia 2d of time stamps in unix epoch, will be used to calculate verified time


public:
    // Metodi pubblici della classe ControlCenter
    ControlCenter();
    void wait();//aspetta il messaggio di "ready" dai droni
    void addDrone(); // adds a drone to the array
    void send_instruction(); //sends the drone an istruction to start a task
    void create_subarea(); // divides the area current(600x600) in sub_areas to be assigned to the individual drones
    // sub areas must be of a max of 250 points - or if sqrt(250) = l, lxl -1 // 
    void divide_tasks(); // assign the subareas to the available drones
    void check_verified(); // checks whether a point has been verified in the last 5 min, if not logs functional requisite has been violeted - checks every point on the grid 
    bool is_verified(); // checks single point
    void run(); //main function, it has the loop on every drone at every istant of time t




    // ...
};

#endif // CONTROL_CENTER_H
