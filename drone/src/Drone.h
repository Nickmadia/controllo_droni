#ifndef DRONE_H
#define DRONE_H
typedef struct {
    int sax; //subarea x 
    int say; //subarea y
    int ny;  // next point of the subarea to start
    int nx;  //
    int dx;  // direction x
    int dy; //usually 0 but changes when doing lawn mowner
    //TODO add dy in everything
} Job;

enum drone_status{
    STARTUP,
    IDLE,
    FLYING,//arrive at start pos
    LAWN_MOWNER, //doing lawn mowner
    WAIT_NEXT_DRONE, // flying to the last pos while other drone arrives
    HOMING, //going home
    CHARGING
};
class Drone {
private:
    // Attributi privati della classe Drone
    int id; // Identificativo unico del drone
    Job * job;
    float battery;// Livello della batteria
    float x;
    float y;
    float velx;
    float vely;
    int lastx;
    int lasty; // only when waiting other drone
    drone_status status;
    float speed; //gets calculated based on how long is time t
    const MAX_DISTANCE = 15000; // max distance in meters the drone can move with it's autonomy of 30 min
    // the drone can never be at distance autonomy left 'd' away from its distance from the center 'c'.  if d>=c then the drone needs to go back to charge

public:
    // Costruttore e metodi pubblici della classe Drone
    Drone(int id);
    void update();
    void charge_drone();
    void start_task(int x, int y); //coordinate della sottoarea
    float get_distance_left();// returns how much distance can the drone run with the current battery level -> (battery_level * autonomia(30min)/60) * speed
    float get_distance_control_center(); // pitagora
    void send_verified_points(); 
    void move_lawn_mowner(); // makes the drone move in the area in a lawn_mowner pattern

    // ...
};
class Swarm {
    private:
        std::vector<Drone> drones;
        int pid;
        redisContext * c; 
        const char * sync_stream = "sync_stream";
        const char * drone_stream = "drone_stream";
        const char * log_stream = "log_stream";

        int block_time = 10000000; //timeout redis msgs
    public:
        void init();
        void init_drones(); // initializes the drones
        void await_sync(); //sync time w/ redis
        void tick(); // time tick
        void send_logs(); // logs
        void shutdown();// close connections
}
#endif // DRONE_H
