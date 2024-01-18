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
Public:
    // Costruttore e metodi pubblici della classe Drone
    Drone(int id);
    void update();
    void chargeDrone();
    void move();
    // ...
};

#endif // DRONE_H
