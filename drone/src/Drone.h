#ifndef DRONE_H
#define DRONE_H

class Drone {
private:
    // Attributi privati della classe Drone
    int drone_id; // Identificativo unico del drone
    float current_speed;
    float battery_level; // Livello della batteria
    bool is_charging; 
    float x;
    float y;
    const MAX_DISTANCE = 15000; // max distance in meters the drone can move with it's autonomy of 30 min
    // the drone can never be at distance autonomy left 'd' away from its distance from the center 'c'.  if d>=c then the drone needs to go back to charge

Public:
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

#endif // DRONE_H
